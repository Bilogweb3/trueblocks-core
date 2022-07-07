/*-------------------------------------------------------------------------------------------
 * qblocks - fast, easily-accessible, fully-decentralized data from blockchains
 * copyright (c) 2016, 2021 TrueBlocks, LLC (http://trueblocks.io)
 *
 * This program is free software: you may redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version. This program is
 * distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details. You should have received a copy of the GNU General
 * Public License along with this program. If not, see http://www.gnu.org/licenses/.
 *-------------------------------------------------------------------------------------------*/
#include "options.h"
#include "bloomwrite.h"

//--------------------------------------------------------------------------
bool COptions::scrape_blocks(void) {
    string_q tmpStagingFn = indexFolder_staging + "000000000-temp.txt";
    tmpStagingStream.open(tmpStagingFn, ios::out | ios::trunc);
    if (!tmpStagingStream.is_open()) {
        LOG_WARN("Could not open temporary staging file.");
        return false;
    }

    blaze_ripe = (meta.client < unripe_dist ? 0 : meta.client - unripe_dist);
    blaze_start = max(meta.ripe, max(meta.staging, meta.finalized)) + 1;
    if ((blaze_start + block_cnt) > meta.client)
        block_cnt = (meta.client - blaze_start);
    if (blaze_start > meta.client) {
        LOG_INFO("The index (", blaze_start, ") is ahead of the chain (", meta.client, ").");
        return false;
    }

    ostringstream blazeCmd;
    blazeCmd << "chifra scrape run --blaze ";
    blazeCmd << "--start_block " << blaze_start << " ";
    blazeCmd << "--ripe_block " << blaze_ripe << " ";
    blazeCmd << "--block_cnt " << block_cnt << " ";
    blazeCmd << "--block_chan_cnt " << block_chan_cnt << " ";
    blazeCmd << "--addr_chan_cnt " << addr_chan_cnt << " ";
    blazeCmd << "--chain " << getChain() << " ";
    blazeCmd << (verbose ? ("--verbose " + uint_2_Str(verbose)) : "");
    // LOG_INFO(bWhite, blazeCmd.str(), cOff);

    if (system(blazeCmd.str().c_str()) != 0) {
        cleanFolder(indexFolder_ripe);
        static bool failed_already = false;
        if (!failed_already) {
            failed_already = true;
            LOG_WARN(cYellow, "Blaze quit without finishing. Retrying...", cOff);
            sleep(3);
            return scrape_blocks();
        }
        LOG_WARN(cYellow, "Blaze quit without finishing twice. Reprocessing...", cOff);
        return false;
    }

    if (isRunning("acctExport")) {
        cleanFolder(indexFolder_ripe);
        LOG_WARN("'chifra export' is running. 'chifra scrape' cannot run at this time...");
        return false;
    }

    string_q stageFn = getLastFileInFolder(indexFolder_staging, false);
    prev_block = path_2_Bn(stageFn);
    nRecsThen = fileSize(stageFn) / asciiAppearanceSize;
    snapped = false;
    if (!forEveryFileInFolder(indexFolder_ripe, copyRipeToStage, this)) {
        if (!snapped) {
            LOG_WARN("copyRipeToStage failed...replaying...");
        }
        cleanFolder(indexFolder_unripe);
        cleanFolder(indexFolder_ripe);
        tmpStagingStream.close();
        ::remove(tmpStagingFn.c_str());
        return snapped;
    }
    tmpStagingStream.close();

    if (!stage_chunks(tmpStagingFn))
        return false;
    if (!isTestMode())
        freshenTimestampsAppend(blaze_start, block_cnt);
    report();
    if (nRecsNow <= apps_per_chunk)
        return true;
    return write_chunks(apps_per_chunk, false /* snapped */);
}

//--------------------------------------------------------------------------
bool copyRipeToStage(const string_q& path, void* data) {
    if (endsWith(path, '/')) {
        return forEveryFileInFolder(path + "*", copyRipeToStage, data);

    } else {
        COptions* opts = reinterpret_cast<COptions*>(data);

        blknum_t bn = path_2_Bn(path);
        bool allow = opts->allow_missing;
        bool sequential = (opts->prev_block + 1) == bn;
        bool less_than = (opts->prev_block < bn);
        if (opts->prev_block != 0 && ((!allow && !sequential) || (allow && !less_than))) {
            LOG_WARN("Current file (", path, ") does not sequentially follow previous file ", opts->prev_block, ".");
            return false;
        }

        ifstream inputStream(path, ios::in);
        if (!inputStream.is_open()) {
            LOG_WARN("Could not open input stream ", path);
            return false;
        }

        lockSection();
        opts->tmpStagingStream << inputStream.rdbuf();
        opts->tmpStagingStream.flush();
        inputStream.close();
        ::remove(path.c_str());
        opts->prev_block = bn;
        unlockSection();

        if (opts->isSnapToGrid(bn)) {
            string_q tmpStagingFn = (indexFolder_staging + "000000000-temp.txt");
            if (!opts->stage_chunks(tmpStagingFn)) {
                LOG_WARN("opts->stage_chunks(" + tmpStagingFn + ") returned false");
                return false;
            }
            opts->report();
            blknum_t nRecords = fileSize(opts->newStage) / asciiAppearanceSize;
            blknum_t chunkSize = min(nRecords, opts->apps_per_chunk);
            opts->write_chunks(chunkSize, true /* snapped */);
            opts->snapped = true;
            return false;
        }
    }
    return !shouldQuit();
}

//---------------------------------------------------------------------------------------------------
bool COptions::write_chunks(blknum_t chunkSize, bool snapped) {
    blknum_t nRecords = fileSize(newStage) / asciiAppearanceSize;
    while ((snapped || nRecords > chunkSize) && !shouldQuit()) {
        lockSection();

        CStringArray lines;
        lines.reserve(nRecords + 100);
        asciiFileToLines(newStage, lines);

        string_q prvBlock;
        size_t loc = NOPOS;
        for (uint64_t record = (chunkSize - 1); record < lines.size() && loc == NOPOS; record++) {
            CStringArray pParts;
            explode(pParts, lines[record], '\t');
            if (prvBlock != pParts[1]) {
                if (!prvBlock.empty())
                    loc = record - 1;
                prvBlock = pParts[1];
            }
        }

        if (loc == NOPOS) {
            loc = lines.size() ? lines.size() - 1 : 0;
        }

        CStringArray remainingLines;
        remainingLines.reserve(chunkSize + 100);
        if (lines.size() > 0) {
            CStringArray consolidatedLines;
            consolidatedLines.reserve(lines.size());
            for (uint64_t record = 0; record <= loc; record++) {
                if (countOf(lines[record], '\t') != 2) {
                    LOG_WARN("Found a record with less than two tabs.");
                    LOG_WARN("preceeding line:\t[", ((record > 0) ? lines[record - 1] : "N/A"), "]");
                    LOG_WARN("offending line:\t[", lines[record], "]");
                    LOG_WARN("following line:\t[", ((record < loc) ? lines[record + 1] : "N/A"), "]");
                    return false;
                }
                consolidatedLines.push_back(lines[record]);
            }

            if (consolidatedLines.size() > 0) {
                CStringArray p1;
                explode(p1, consolidatedLines[0], '\t');
                CStringArray p2;
                explode(p2, consolidatedLines[consolidatedLines.size() - 1], '\t');

                sort(consolidatedLines.begin(), consolidatedLines.end());
                string_q chunkId = p1[1] + "-" + p2[1];
                string_q chunkPath = indexFolder_finalized + chunkId + ".bin";
                LOG_INFO("Writing...", string_q(80, ' '), "\r");
                writeIndexAsBinary(chunkPath, consolidatedLines);
                ostringstream os;
                os << "Wrote " << consolidatedLines.size() << " records to " << cTeal << relativize(chunkPath);
                if (snapped && (lines.size() - 1 == loc)) {
                    os << cYellow << " (snapped to " << snap_to_grid << " blocks)";
                }
                os << cOff;
                LOG_INFO(os.str());

                loc++;

                for (uint64_t record = loc; record < lines.size(); record++)
                    remainingLines.push_back(lines[record]);

                ::remove(newStage.c_str());
            }
        }

        if (remainingLines.size()) {
            linesToAsciiFile(newStage, remainingLines);
        }
        unlockSection();

        nRecords = fileSize(newStage) / asciiAppearanceSize;
        if (snapped)
            snapped = nRecords > 0;
        chunkSize = min(apps_per_chunk, nRecords);
    }

    return !shouldQuit();
}

//--------------------------------------------------------------------------
bool COptions::stage_chunks(const string_q& tmpFn) {
    string_q prevStage = getLastFileInFolder(indexFolder_staging, false);
    newStage = indexFolder_staging + padNum9(prev_block) + ".txt";
    if (prevStage == newStage) {
        return !shouldQuit();
    }

    string_q tmpFile = indexFolder + "temp.txt";
    if (prevStage != tmpFn) {
        if (!appendFile(tmpFile /* to */, prevStage /* from */)) {
            cleanFolder(indexFolder_unripe);
            cleanFolder(indexFolder_ripe);
            ::remove(tmpFile.c_str());
            ::remove(tmpFn.c_str());
            return false;
        }
    }

    if (!appendFile(tmpFile /* to */, tmpFn /* from */)) {
        cleanFolder(indexFolder_unripe);
        cleanFolder(indexFolder_ripe);
        ::remove(tmpFile.c_str());
        ::remove(tmpFn.c_str());
        return false;
    }

    lockSection();
    ::rename(tmpFile.c_str(), newStage.c_str());
    ::remove(tmpFn.c_str());
    if (fileExists(prevStage) && prevStage != newStage)
        ::remove(prevStage.c_str());
    unlockSection();
    return !shouldQuit();
}

//---------------------------------------------------------------------------------------------------
bool COptions::report(void) {
    nRecsNow = fileSize(newStage) / asciiAppearanceSize;
    blknum_t found = nRecsNow - nRecsThen;
    if (!found) {
        LOG_INFO("No new blocks...", string_q(80, ' '), "\r");
        return true;
    }

    blknum_t need = apps_per_chunk >= nRecsNow ? apps_per_chunk - nRecsNow : 0;
    double pct = double(nRecsNow) / double(apps_per_chunk);
    double pBlk = double(found) / double(block_cnt);

    string_q result = "Block {0}: have {1} addrs of {2} ({3}). Need {4} more. Found {5} records ({6}).";
    replace(result, "{0}", "{" + uint_2_Str(blaze_start + block_cnt) + "}");
    replace(result, "{1}", "{" + uint_2_Str(nRecsNow) + "}");
    replace(result, "{2}", "{" + uint_2_Str(apps_per_chunk) + "}");
    replace(result, "{3}", "{" + double_2_Str(pct * 100.00, 1) + "%}");
    replace(result, "{4}", "{" + uint_2_Str(need) + "}");
    replace(result, "{5}", "{" + uint_2_Str(found) + "}");
    replace(result, "{6}", "{" + double_2_Str(pBlk, 2) + " apps/blk}");
    replaceAll(result, "{", cGreen);
    replaceAll(result, "}", cOff);
    LOG_INFO(result);
    return true;
}

//----------------------------------------------------------------
bool writeIndexAsBinary(const string_q& outFn, const CStringArray& lines) {
    // ASSUMES THE ARRAY IS SORTED!

    ASSERT(!fileExists(outFn));
    string_q tmpFile2 = substitute(outFn, ".bin", ".tmp");

    address_t prev;
    uint32_t offset = 0, nAddrs = 0, cnt = 0;
    CIndexedAppearanceArray blockTable;

    // TODO: BOGUS - MANIFEST WRITING HASH INTO INDEX
    hashbytes_t hash = hash_2_Bytes(padLeft(keccak256(manifestVersion), 64, '0'));

    CArchive archive(WRITING_ARCHIVE);
    if (!archive.Lock(tmpFile2, modeWriteCreate, LOCK_NOWAIT)) {
        LOG_ERR("Could not lock index file: ", tmpFile2);
        return false;
    }

    archive.Seek(0, SEEK_SET);  // write the header even though it's not fully detailed to preserve the space
    archive.Write(MAGIC_NUMBER);
    archive.Write(hash.data(), hash.size(), sizeof(uint8_t));
    archive.Write(nAddrs);
    archive.Write((uint32_t)blockTable.size());  // not accurate yet

    CBloomFilterWrite bloomFilter;
    for (size_t l = 0; l < lines.size(); l++) {
        string_q line = lines[l];
        ASSERT(countOf(line, '\t') == 2);
        CStringArray parts;
        explode(parts, line, '\t');
        CIndexedAppearance rec(parts[1], parts[2]);
        blockTable.push_back(rec);
        if (!prev.empty() && parts[0] != prev) {
            bloomFilter.addToSet(prev);
            addrbytes_t bytes = addr_2_Bytes(prev);
            archive.Write(bytes.data(), bytes.size(), sizeof(uint8_t));
            archive.Write(offset);
            archive.Write(cnt);
            offset += cnt;
            cnt = 0;
            nAddrs++;
        }
        cnt++;
        prev = parts[0];
    }

    // The above algo always misses the last address, so we add it here
    bloomFilter.addToSet(prev);
    addrbytes_t bytes = addr_2_Bytes(prev);
    archive.Write(bytes.data(), bytes.size(), sizeof(uint8_t));
    archive.Write(offset);
    archive.Write(cnt);
    nAddrs++;

    for (auto record : blockTable) {
        archive.Write(record.blk);
        archive.Write(record.txid);
    }

    archive.Seek(0, SEEK_SET);  // re-write the header now that we have full data
    archive.Write(MAGIC_NUMBER);
    archive.Write(hash.data(), hash.size(), sizeof(uint8_t));
    archive.Write(nAddrs);
    archive.Write((uint32_t)blockTable.size());
    archive.Release();

    // We've built the data in a temporary file. We do this in case we're interrupted during the building of the
    // data so it's not corrupted. In this way, we only move the data to its final resting place once. It's safer.
    string_q bloomFile = substitute(substitute(outFn, "/finalized/", "/blooms/"), ".bin", ".bloom");
    lockSection();                            // disallow control+c
    bloomFilter.writeBloomFilter(bloomFile);  // write the bloom file
    copyFile(tmpFile2, outFn);                // move the index file
    ::remove(tmpFile2.c_str());               // remove the tmp file
    unlockSection();

    string_q range = substitute(substitute(outFn, indexFolder_finalized, ""), indexFolder_blooms, "");
    range = substitute(substitute(range, ".bin", ""), ".bloom", "");
    appendToAsciiFile(cacheFolder_tmp + "chunks_created.txt", range + "\n");

    return !shouldQuit();
}

// TODO: BOGUS THIS DOES NOT WORK
namespace qblocks {
extern bool establishTsFile(void);
}
//-----------------------------------------------------------------------
bool freshenTimestampsAppend(blknum_t firstBlock, blknum_t nBlocks) {
    // This routine is called after Blaze finishes. Blaze will have written a text file containing
    // blockNumber,timestamp pairs that it encountered during it's pass. The job of this
    // routine is to append timestamps to the timestamps database up to but not
    // including firstBlock + nBlocks (so 0 and 1, if firstBlock == 0 and nBlocks == 2)
    // Note that the text file may not contain every block. In fact, on many chains
    // it does not. This routine has to recitfy that. Plus -- if this routine does not
    // complete, it may have to start prior to firstBlock the next time, so we have to
    // check the invariant: sizeof(file) == bn[last].Bn * 2 and bn[i] == i.

    if (isTestMode())
        return true;

    if (!establishTsFile())
        return false;

    if (fileExists(indexFolderBin_ts + ".lck")) {  // it's being updated elsewhere
        LOG_ERR("Timestamp file ", indexFolderBin_ts, " is locked. Cannot update. Re-running...");
        ::remove((indexFolderBin_ts + ".lck").c_str());
        return false;
    }

    // If we're already there, we're finished
    blknum_t lastBlock = firstBlock + nBlocks;
    size_t nRecords = ((fileSize(indexFolderBin_ts) / sizeof(uint32_t)) / 2);

    LOG_INFO("\n");
    LOG_INFO(string_q(120, '-'), "\n", string_q(120, '-'));
    LOG_INFO("ENTERING WITH firstBlock: ", firstBlock, "\tnBlocks:   ", nBlocks);
    LOG_INFO("              nRecords:   ", firstBlock, "\tlastBlock: ", nBlocks);

    if (nRecords >= lastBlock) {
        LOG_INFO("LEAVING EARLY");
        return true;
    }
    LOG_INFO("PROCESSING");

    // We always start at one less than the number of blocks already in the file
    // (i.e., if there's two we need to add block two next.)
    firstBlock = nRecords;

    // We need to fill blocks firstBlock to lastBlock (non-inclusive). Make note
    // of which ones were processed by Blaze
    string_q blazeTsFilename = cacheFolder_tmp + "tempTsFile.txt";
    CStringArray lines;
    asciiFileToLines(blazeTsFilename, lines);
    // The strings are left-padded with zeros, so we can sort them as strings
    sort(lines.begin(), lines.end());

    typedef map<blknum_t, timestamp_t> bn_ts_map_t;
    bn_ts_map_t theMap;
    for (auto line : lines) {
        if (shouldQuit())
            break;
        CStringArray parts;
        explode(parts, line, '-');
        if (parts.size() == 2) {
            theMap[str_2_Uint(parts[0])] = str_2_Int(parts[1]);
        } else {
            LOG_ERR("Line without dash (-) in temporarty timestamp file. Cannot continue.");
            break;
        }
    }

    for (blknum_t bn = firstBlock; bn < (firstBlock + nBlocks) && !shouldQuit(); bn++) {
        if (theMap[bn] == 0) {
            CBlock block;
            getBlockHeader(block, bn);
            theMap[bn] = block.timestamp;
        }
        // LOG_INFO(bn, "\t", theMap[bn]);
    }

    lockSection();

    // LOG_INFO("Updating");
    CArchive file(WRITING_ARCHIVE);
    if (!file.Lock(indexFolderBin_ts, modeWriteAppend, LOCK_NOWAIT)) {
        LOG_ERR("Failed to open ", indexFolderBin_ts);
        unlockSection();
        return false;
    }

    for (auto item : theMap) {
        if (shouldQuit())
            break;
        file << ((uint32_t)item.first) << ((uint32_t)item.second);
        file.flush();
        ostringstream post;
        post << " (" << (lastBlock - item.first);
        post << " " << item.second << " - " << ts_2_Date(item.second).Format(FMT_EXPORT) << ")";
        post << "             \r";
        LOG_INFO(UPDATE, item.first, lastBlock, post.str());
    }
    file.Release();
    unlockSection();
    return true;
}

/*
    CBlock block;
    for (blknum_t bn = nRecords; bn < (minBlock + 1) && !shouldQuit(); bn++) {
        block = CBlock();  // reset
        getBlockHeader(block, bn);
        if (block.timestamp < blockZeroTs()) {
            LOG_ERR("Bad data when requesting block '", bn, "'. ", block.Format("[{BLOCKNUMBER}].[{TIMESTAMP}]"),
                    " Cannot continue.");
            file.Release();
            unlockSection();
            return false;
        } else {
            file << ((uint32_t)block.blockNumber) << ((uint32_t)block.timestamp);
            file.flush();
            ostringstream post;
            post << " (" << ((minBlock + 1) - block.blockNumber);
            post << " " << block.timestamp << " - " << ts_2_Date(block.timestamp).Format(FMT_EXPORT) << ")";
            post << "             \r";
            LOG_PROGRESS(UPDATE, block.blockNumber, minBlock, post.str());
        }
    }
*/
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
#include "node.h"

namespace qblocks {

//-------------------------------------------------------------------------
bool freshenAndLoad(void) {
    if (!isTestMode() && !isApiMode())
        freshenTimestamps(getBlockProgress().client);  // opens, freshens, and closes the file
    if (!loadTimestamps(&expContext().tsMemMap, expContext().tsCnt)) {
        LOG_WARN("Could not load timestamp file");
        return false;
    }
    LOG4("Loaded timestamp file");
    return true;
}

//-------------------------------------------------------------------------
blknum_t getTimestampBlockAt(blknum_t blk) {
    if (!expContext().tsMemMap)
        if (!freshenAndLoad())
            return 0;
    if (expContext().tsMemMap && blk < expContext().tsCnt)
        return expContext().tsMemMap[(blk * 2)];
    return 0;
}

//-------------------------------------------------------------------------
timestamp_t getTimestampAt(blknum_t blk) {
    if (!expContext().tsMemMap)
        if (!freshenAndLoad())
            return 0;
    if (expContext().tsMemMap && blk < expContext().tsCnt)
        return timestamp_t(expContext().tsMemMap[(blk * 2) + 1]);
    return 0;
}

//-------------------------------------------------------------------------
size_t nTimestamps(void) {
    return ((fileSize(tsIndex) / sizeof(uint32_t)) / 2);
}

//-----------------------------------------------------------------------
bool establishTsFile(void) {
    if (fileExists(tsIndex))
        return true;

    establishFolder(indexFolder);

    string_q zipFile = configPath("ts.bin.gz");
    time_q zipDate = fileLastModifyDate(zipFile);
    time_q tsDate = fileLastModifyDate(tsIndex);

    if (zipDate > tsDate) {
        ostringstream cmd;
        cmd << "cd \"" << indexFolder << "\" ; ";
        cmd << "cp \"" << zipFile << "\" . ; ";
        cmd << "gunzip ts.bin.gz";
        string_q result = doCommand(cmd.str());
        LOG_INFO(result);
        // The original zip file still exists
        ASSERT(fileExists(zipFile));
        // The new timestamp file exists
        ASSERT(fileExists(tsIndex));
        // The copy of the zip file does not exist
        ASSERT(!fileExists(tsIndex + ".gz"));
        return fileExists(tsIndex);
    }

    // starts at zero...
    return true;
}

//-----------------------------------------------------------------------
bool freshenTimestamps(blknum_t minBlock) {
    if (isTestMode())
        return true;

    // LOG_INFO("Not test mode. minBlock: ", minBlock);
    if (!establishTsFile())
        return false;

    // LOG_INFO("Established ts file");
    size_t nRecords = ((fileSize(tsIndex) / sizeof(uint32_t)) / 2);
    if (nRecords >= minBlock)
        return true;

    // LOG_INFO("Found ", nRecords, " records");
    if (fileExists(tsIndex + ".lck")) {  // it's being updated elsewhere
        LOG_ERR("Timestamp file ", tsIndex, " is locked. Cannot update.");
        return false;
    }

    // LOG_INFO("Updating");
    CArchive file(WRITING_ARCHIVE);
    if (!file.Lock(tsIndex, modeWriteAppend, LOCK_NOWAIT)) {
        LOG_ERR("Failed to open ", tsIndex);
        return false;
    }

    if (nRecords == 0) {
        file << (uint32_t)0 << (uint32_t)blockZeroTs;
        file.flush();
        LOG_INFO(padNum9(0), "\t", blockZeroTs, "\t", ts_2_Date(blockZeroTs).Format(FMT_EXPORT), "          \r");
        nRecords++;
    }

    CBlock block;
    for (blknum_t bn = nRecords; bn <= minBlock; bn++) {
        block = CBlock();  // reset
        getBlock_header(block, bn);
        file << ((uint32_t)block.blockNumber) << ((uint32_t)block.timestamp);
        file.flush();
        ostringstream post;
        post << " (" << block.timestamp << " - " << ts_2_Date(block.timestamp).Format(FMT_EXPORT) << ")"
             << "\r";
        ostringstream pre;
        pre << "Updating " << (minBlock - block.blockNumber) << " timestamps ";
        LOG_PROGRESS(pre.str(), block.blockNumber, minBlock, post.str());
    }
    cerr << "\r" << string_q(150, ' ') << "\r";
    cerr.flush();

    file.Release();
    return true;
}

//-----------------------------------------------------------------------
bool loadTimestamps(uint32_t** theArray, size_t& cnt) {
    static CMemMapFile file;
    if (file.is_open())
        file.close();

    if (!establishTsFile())
        return false;

    // Order matters.
    // User may call us with a NULL array pointer, but we still want to file 'cnt'
    cnt = ((fileSize(tsIndex) / sizeof(uint32_t)) / 2);
    if (theArray == NULL)
        return cnt;

    file.open(tsIndex);
    if (file.isValid())
        *theArray = (uint32_t*)file.getData();  // NOLINT

    return true;
}

//-------------------------------------------------------------------------
bool forEveryTimestamp(BLOCKVISITFUNC func, void* data) {
    if (!func)
        return false;

    size_t n = nTimestamps();
    for (size_t index = 0; index < n; index++) {
        CBlock block;
        block.blockNumber = getTimestampBlockAt(index);
        block.timestamp = getTimestampAt(index);
        bool ret = (*func)(block, data);
        if (!ret)
            return false;
    }
    return true;
}

}  // namespace qblocks

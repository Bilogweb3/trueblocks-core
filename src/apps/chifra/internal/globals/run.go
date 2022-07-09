// Copyright 2021 The TrueBlocks Authors. All rights reserved.
// Use of this source code is governed by a license that can
// be found in the LICENSE file.

package globals

import (
	"bufio"
	"bytes"
	"fmt"
	"io"
	"os"
	"os/exec"
	"sync"

	"github.com/TrueBlocks/trueblocks-core/src/apps/chifra/pkg/config"
)

func (opts *GlobalOptions) PassItOn(path, chain, cmdLine string, envIn []string) error {
	options := cmdLine
	options += opts.ToCmdLine()

	var wg sync.WaitGroup
	wg.Add(2)

	var env config.ConfigEnv
	env.Chain = chain
	env.ConfigPath = config.GetPathToRootConfig()
	env.DefaultChain = config.GetDefaultChain()
	env.CachePath = config.GetPathToCache(env.Chain)
	env.ChainConfigPath = config.GetPathToChainConfig(env.Chain) // order matters
	env.IndexPath = config.GetPathToIndex(env.Chain)             // order matters
	env.RpcProvider = config.GetRpcProvider(env.Chain)
	envStr := env.ToCSV()

	cmd := exec.Command(config.GetPathToCommands(path), options)
	cmd.Env = append(os.Environ(), "FROM_CHIFRA=true")
	cmd.Env = append(cmd.Env, "TB_CONFIG_ENV="+envStr)
	for _, e := range envIn {
		cmd.Env = append(cmd.Env, e)
	}
	if os.Getenv("TEST_MODE") == "true" {
		cmd.Env = append(cmd.Env, "TEST_MODE=true")
	}

	stderrPipe, err := cmd.StderrPipe()
	if err != nil {
		fmt.Fprintf(os.Stderr, "%s", err)
	} else {
		go func() {
			ScanForProgress2(stderrPipe, func(msg string) {
			})
			wg.Done()
		}()
	}

	// TODO: BOGUS - RETURN VALUE FROM BLAZE
	// cmd := exec.Command("git", "blub")
	// if err := cmd.Start(); err != nil {
	// 	log.Fatalf("cmd.Start: %v", err)
	// }
	// if err := cmd.Wait(); err != nil {
	// 	if exiterr, ok := err.(*exec.ExitError); ok {
	// 		if status, ok := exiterr.Sys().(syscall.WaitStatus); ok {
	// 			log.Printf("Exit Status: %d", status.ExitStatus())
	// 		}
	// 	} else {
	// 		log.Fatalf("cmd.Wait: %v", err)
	// 	}
	// }

	// TODO: BOGUS - RETURN VALUE FROM BLAZE
	// returnVal := int64(0)
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		fmt.Fprintf(os.Stderr, "%s", err)
	} else {
		go func() {
			cmd.Start()
			scanner := bufio.NewScanner(stdout)
			buf := make([]byte, 1024*1024)
			scanner.Buffer(buf, 1024*1024)
			for scanner.Scan() {
				m := scanner.Text()
				// TODO: BOGUS - RETURN VALUE FROM BLAZE
				// returnVal, _ = strconv.ParseInt(m, 10, 32)
				fmt.Println(m)
			}
			wg.Done()
		}()
	}
	wg.Wait()
	cmd.Wait()
	// fmt.Fprintf(os.Stderr, "Calling: TB_CONFIG_ENV=\"%s\" %s %s\n", envStr, config.GetPathToCommands(path), options)
	// time.Sleep(4 * time.Second)
	// TODO: BOGUS - RETURN VALUE FROM BLAZE
	// if returnVal != 0 {
	//	msg := fmt.Sprintf("call returned %d", returnVal)
	//	return errors.New(msg)
	// }
	return nil
}

// dropNL2 drops new line characters (\n) from the progress stream
func dropNL2(data []byte) []byte {
	if len(data) > 0 && data[len(data)-1] == '\n' {
		return data[0 : len(data)-1]
	}
	return data
}

// ScanProgressLine2 looks for "lines" that end with `\r` not `\n` like usual
func ScanProgressLine2(data []byte, atEOF bool) (advance int, token []byte, err error) {
	if atEOF && len(data) == 0 {
		return 0, nil, nil
	}
	if i := bytes.IndexByte(data, '\n'); i >= 0 {
		fmt.Fprintf(os.Stderr, "%s\n", string(data[0:i]))
		return i + 1, data[0:i], nil
	}
	if i := bytes.IndexByte(data, '\r'); i >= 0 {
		fmt.Fprintf(os.Stderr, "%s\r", string(data[0:i]))
		return i + 1, dropNL2(data[0:i]), nil
	}
	return bufio.ScanLines(data, atEOF)
}

// ScanForProgress2 watches stderr and picks of progress messages
func ScanForProgress2(stderrPipe io.Reader, fn func(string)) {
	scanner := bufio.NewScanner(stderrPipe)
	scanner.Split(ScanProgressLine2)
	for scanner.Scan() {
		// we've already printed the token
	}
	if err := scanner.Err(); err != nil {
		fmt.Println("TB: Error while reading stderr -- ", err)
	}
}

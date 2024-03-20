// Copyright 2024 The TrueBlocks Authors. All rights reserved.
// Use of this source code is governed by a license that can
// be found in the LICENSE file.
/*
 * Parts of this file were generated with makeClass --run. Edit only those parts of
 * the code inside of 'EXISTING_CODE' tags.
 */

package sdk

import (
	"io"
	"net/url"

	{{.Pkg}} "github.com/TrueBlocks/trueblocks-core/src/apps/chifra/internal/{{.Lower}}"
	outputHelpers "github.com/TrueBlocks/trueblocks-core/src/apps/chifra/pkg/output/helpers"
)

// {{.Proper}} provides an interface to the command line chifra {{.Lower}} through the SDK.
func {{.Proper}}(w io.Writer, values url.Values) error {
	{{.Pkg}}.ResetOptions(sdkTestMode)
	opts := {{.Pkg}}.{{.Proper}}FinishParseInternal(w, values)
	outputHelpers.EnableCommand("{{.Lower}}", true)
	// EXISTING_CODE
	// EXISTING_CODE
	outputHelpers.InitJsonWriterApi("{{.Lower}}", w, &opts.Globals)
	err := opts.{{.Proper}}Internal()
	outputHelpers.CloseJsonWriterIfNeededApi("{{.Lower}}", err, &opts.Globals)

	return err
}

// EXISTING_CODE
// EXISTING_CODE

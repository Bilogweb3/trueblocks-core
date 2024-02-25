// Copyright 2024 The TrueBlocks Authors. All rights reserved.
// Use of this source code is governed by a license that can
// be found in the LICENSE file.
/*
 * Parts of this file were generated with makeClass --run. Edit only those parts of
 * the code inside of 'EXISTING_CODE' tags.
 */

package sdk

import (
	// EXISTING_CODE
	"io"
	"net/url"

	receipts "github.com/TrueBlocks/trueblocks-core/src/apps/chifra/sdk"
	// EXISTING_CODE
)

type ReceiptsOptions struct {
	TransactionIds []string
	Articulate     bool
	Globals

	// EXISTING_CODE
	// EXISTING_CODE
}

// Receipts implements the chifra receipts command for the SDK.
func (opts *ReceiptsOptions) Receipts(w io.Writer) error {
	values := make(url.Values)

	// EXISTING_CODE
	for _, v := range opts.TransactionIds {
		values.Add("transactions", v)
	}
	if opts.Articulate {
		values.Set("articulate", "true")
	}
	// EXISTING_CODE
	opts.Globals.mapGlobals(values)

	return receipts.Receipts(w, values)
}

// EXISTING_CODE
// EXISTING_CODE


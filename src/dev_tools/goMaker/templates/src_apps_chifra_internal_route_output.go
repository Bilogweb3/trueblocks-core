// Copyright 2021 The TrueBlocks Authors. All rights reserved.
// Use of this source code is governed by a license that can
// be found in the LICENSE file.
/*
 * Parts of this file were generated with makeClass --run. Edit only those parts of
 * the code inside of 'EXISTING_CODE' tags.
 */

package {{.Route}}Pkg

// EXISTING_CODE
// EXISTING_CODE

// Run{{.Proper}} handles the {{.Route}} command for the command line. Returns error only as per cobra.
func Run{{.Proper}}(cmd *cobra.Command, args []string) error {
	opts := {{.Lower}}FinishParse(args)
	outputHelpers.EnableCommand("{{.Route}}", true)
	// EXISTING_CODE
	// EXISTING_CODE
	outputHelpers.SetWriterForCommand("{{.Route}}", &opts.Globals)
	return opts.{{.Proper}}Internal()
}

// Serve{{.Proper}} handles the {{.Route}} command for the API. Returns an error.
func Serve{{.Proper}}(w http.ResponseWriter, r *http.Request) error {
	opts := {{.Lower}}FinishParseApi(w, r)
	outputHelpers.EnableCommand("{{.Route}}", true)
	// EXISTING_CODE
	// EXISTING_CODE
	outputHelpers.InitJsonWriterApi("{{.Route}}", w, &opts.Globals)
	err := opts.{{.Proper}}Internal()
	outputHelpers.CloseJsonWriterIfNeededApi("{{.Route}}", err, &opts.Globals)
	return err
}

// {{.Proper}}Internal handles the internal workings of the {{.Route}} command.  Returns an error.
func (opts *{{.Proper}}Options) {{.Proper}}Internal() error {
	var err error
	if err = opts.validate{{.Proper}}(); err != nil {
		return err
	}

	timer := logger.NewTimer()
	msg := "chifra {{.Route}}"
	// EXISTING_CODE
	// EXISTING_CODE
	timer.Report(msg)

	return err
}

// Get{{.Proper}}Options returns the options for this tool so other tools may use it.
func Get{{.Proper}}Options(args []string, g *globals.GlobalOptions) *{{.Proper}}Options {
	ret := {{.Route}}FinishParse(args)
	if g != nil {
		ret.Globals = *g
	}
	return ret
}

// EXISTING_CODE
// EXISTING_CODE

//
// Copyright (C) 2025-2026 Free Software Foundation, Inc.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Written by Bruno Haible.

// This program passes an input to an ollama instance and prints the response.

package main

import (
	"bufio"
	"bytes"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"net/http"
	"os"
	"strings"
)

func main() {
	// Command-line option processing.
	// Note: This is incompatible with GNU conventions.
	// <https://pkg.go.dev/flag#hdr-Command_line_flag_syntax> says
	// "Flag parsing stops just before the first non-flag argument
	//  ("-" is a non-flag argument) or after the terminator "--"."
	// which means that options after non-options are not reordered
	// like they are by GNU getopt.
	// The common workaround is to use https://github.com/spf13/pflag
	// instead.

	url_option :=     flag.String("url",   "http://localhost:11434", "the ollama server's URL")

	// Clumsy code is needed when we want an option of type string
	// that has no default. The 'flag' package's String and StringVar
	// functions reject a default value of nil, pushing developers
	// into confusing an option with an empty string value with an
	// omitted option.
	var model_option *string = nil
	                  flag.Func(  "model",                           "the model to use",
			    func (s string) error {
			      model_option = &s
			      return nil
			    })

	do_help_option := flag.Bool  ("help",  false,                    "this help text")

	flag.Parse()

	if *do_help_option {
		fmt.Println("Usage: spit [OPTION...]")
		fmt.Println()
		fmt.Println("Passes standard input to an ollama instance and prints the response.")
		fmt.Println()
		fmt.Println("Options:")
		fmt.Println("      --url      Specifies the ollama server's URL.")
		fmt.Println("      --model    Specifies the model to use.")
		fmt.Println()
		fmt.Println("Informative output:")
		fmt.Println()
		fmt.Println("      --help     Show this help text.")
		os.Exit(0)
	}

	if model_option == nil {
		fmt.Fprintln(os.Stderr, "spit: missing --model option")
		os.Exit(1)
	}

	if len(flag.Args()) > 0 {
		fmt.Fprintln(os.Stderr, "spit: too many arguments")
		fmt.Fprintln(os.Stderr, "'Try 'spit --help' for more information.")
		os.Exit(1)
	}

	// Sanitize URL.
	url := *url_option
	if !strings.HasSuffix(url, "/") {
		url = url + "/"
	}

	model := *model_option

	// Read the contents of standard input.
	allBytes, err := io.ReadAll(os.Stdin)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	input := string(allBytes)

	// Documentation of the ollama API:
	// <https://docs.ollama.com/api/generate>

	// JSON in Go is a pain:
	// 1) There is no way to just create a JSON object and add properties
	//    to it. We are forced to either use a map[string]any (and lose
	//    the advantages of type checking) or create a struct that reflects
	//    the desired shape of the JSON object.
	// 2) In this struct, fields whose name starts with a lowercase letter
	//    are ignored by json.Marshal! Here's the workaround syntax:
	type GeneratePayload struct {
		Model  string `json:"model"`
		Prompt string `json:"prompt"`
	}
	payload := GeneratePayload {
		Model: model,
		Prompt: input,
	}
	payloadAsBytes, err := json.Marshal(payload)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	response, err := http.Post(url + "api/generate",
	                           "application/json", bytes.NewReader(payloadAsBytes))
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
	if response.StatusCode != 200 {
		fmt.Fprintln(os.Stderr, "Status:", response.StatusCode)
	}
	if response.StatusCode >= 400 {
		responseBodyBytes, _ := io.ReadAll(response.Body)
		fmt.Fprintln(os.Stderr, "Body:", string(responseBodyBytes))
		os.Exit(1)
	}

	body := response.Body
	reader := bufio.NewReader(body)
	for {
		line, err := reader.ReadBytes('\n')
		if len(line) == 0 && err != nil {
			break
		}
		var part map[string]any
		if json.Unmarshal(line, &part) == nil {
			fmt.Print(part["response"])
		}
	}
}

/*
 * Local Variables:
 * compile-command: "gccgo -Wall -O2 -o ollama-spit ollama-spit.go"
 * run-command: "echo 'Translate into German: "Welcome to the GNU project!"' | ./ollama-spit --model=ministral-3:14b"
 * End:
 */

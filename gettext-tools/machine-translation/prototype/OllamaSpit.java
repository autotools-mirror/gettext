/*
 * Copyright (C) 2025 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Written by Bruno Haible <bruno@clisp.org>, 2025.
 */

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.net.URI;

// Documentation:
// https://docs.oracle.com/en/java/javase/11/docs/api/java.net.http/java/net/http/package-summary.html
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;

// Documentation:
// https://google.github.io/gson/UserGuide.html
// https://www.javadoc.io/doc/com.google.code.gson/gson/2.8.0/com/google/gson/Gson.html
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

/*
 * This program passes an input to an ollama instance and prints the response.
 */
public class OllamaSpit {

  private static void usage () {
    System.out.println("Usage: spit [OPTION...]");
    System.out.println();
    System.out.println("Passes standard input to an ollama instance and prints the response.");
    System.out.println();
    System.out.println("Options:");
    System.out.println("      --url      Specifies the ollama server's URL.");
    System.out.println("      --model    Specifies the model to use.");
    System.out.println();
    System.out.println("Informative output:");
    System.out.println();
    System.out.println("      --help     Show this help text.");
  }

  public static void main (String[] args) throws IOException, InterruptedException {
    // Command-line option processing.
    String url = "http://localhost:11434";
    String model = null;
    {
      boolean do_help = false;
      int i;
      for (i = 0; i < args.length; i++) {
        String arg = args[i];
        if (arg.equals("--url") || arg.equals("--ur") || arg.equals("--u")) {
          i++;
          if (i == args.length) {
            System.err.println("Spit: missing argument for --url");
            System.exit(1);
          }
          url = args[i];
        } else if (arg.startsWith("--url=")) {
          url = arg.substring(6);
        } else if (arg.equals("--model") || arg.equals("--mode") || arg.equals("--mod") || arg.equals("--mo") || arg.equals("--m")) {
          i++;
          if (i == args.length) {
            System.err.println("Spit: missing argument for --model");
            System.exit(1);
          }
          model = args[i];
        } else if (arg.startsWith("--model=")) {
          model = arg.substring(8);
        } else if (arg.equals("--help")) {
          do_help = true;
        } else if (arg.equals("--")) {
          // Stop option processing
          i++;
          break;
        } else if (arg.startsWith("-")) {
          System.err.println("Spit: unknown option " + arg);
          System.err.println("Try 'Spit --help' for more information.");
          System.exit(1);
        } else
          break;
      }
      if (do_help) {
        usage();
        System.exit(0);
      }
      if (i < args.length) {
        System.err.println("Spit: too many arguments");
        System.err.println("Try 'Spit --help' for more information.");
        System.exit(1);
      }
    }
    if (model == null) {
      System.err.println("Spit: missing --model option");
      System.exit(1);
    }
    // Sanitize URL.
    if (!url.endsWith("/"))
      url = url + "/";

    // Read the contents of standard input.
    String input = new String(System.in.readAllBytes(), StandardCharsets.UTF_8);

    // Documentation of the ollama API:
    // <https://docs.ollama.com/api/generate>

    // Compose the payload.
    JsonObject payload = new JsonObject();
    payload.addProperty("model", model);
    payload.addProperty("prompt", input);
    String payloadAsString = payload.toString();

    // Make the request to the ollama server.
    HttpClient client = HttpClient.newHttpClient();
    HttpRequest request =
      HttpRequest.newBuilder(URI.create(url+"api/generate"))
                 .POST(HttpRequest.BodyPublishers.ofString(payloadAsString))
                 .build();
    HttpResponse<InputStream> response =
      client.send(request, HttpResponse.BodyHandlers.ofInputStream());
    InputStream responseStream = response.body();
    if (response.statusCode() != 200) {
      System.err.println("Status: "+response.statusCode());
    }
    if (response.statusCode() >= 400) {
      System.err.print("Body: ");
      responseStream.transferTo(System.err);
      System.err.println();
      System.exit(1);
    }
    int bufSize = 4096;
    byte[] buffer = new byte[bufSize];
    int bufStart = 0;
    int bufEnd = 0;
    for (;;) {
      // Read a line when available.
      // But don't force reading more than one line.
      String line;
      {
        // Boy, this is complicated code. There should really be a helper
        // method on InputStream for this purpose.
        int i;
        for (i = bufStart; i < bufEnd; i++) {
          if (buffer[i] == (byte)'\n')
            break;
        }
        for (;;) {
          if (i < bufEnd) {
            // An entire line in buffer.
            i++;
            line = new String(buffer, bufStart, i-bufStart, StandardCharsets.UTF_8);
            bufStart = i;
            break;
          }
          // We need to read something from the stream.
          int avail = responseStream.available();
          if (avail == 0)
            // If nothing is available, read 1 byte, even if that needs to block.
            avail = 1;
          if ((bufEnd-bufStart) + avail > bufSize) {
            // Grow the buffer.
            int newBufSize = (bufEnd-bufStart) + avail;
            if (newBufSize < 2*bufSize)
              newBufSize = 2*bufSize;
            byte[] newBuffer = new byte[newBufSize];
            System.arraycopy(buffer, bufStart, newBuffer, 0, bufEnd-bufStart);
            bufSize = newBufSize;
            buffer = newBuffer;
            bufEnd = bufEnd-bufStart;
            bufStart = 0;
          } else {
            // We can keep the buffer, but may need to move the contents to the
            // front.
            if (bufEnd + avail > bufSize) {
              System.arraycopy(buffer, bufStart, buffer, 0, bufEnd-bufStart);
              bufEnd = bufEnd-bufStart;
              bufStart = 0;
            }
          }
          // Now bufEnd + avail <= bufSize.
          int nBytesRead = responseStream.readNBytes(buffer, bufEnd, avail);
          if (nBytesRead == 0) {
            // We're at EOF.
            if (bufEnd == bufStart)
              return;
            // Convert the last line (without terminating newline).
            line = new String(buffer, bufStart, bufEnd-bufStart, StandardCharsets.UTF_8);
            break;
          }
          // We have read at least one byte. Look if we have a complete line now.
          int oldBufEnd = bufEnd;
          bufEnd += nBytesRead;
          for (i = oldBufEnd; i < bufEnd; i++) {
            if (buffer[i] == (byte)'\n')
              break;
          }
        }
      }
      // We have a line now. It should contain a single JSON object.
      JsonElement part = JsonParser.parseString(line);
      if (part.isJsonObject()) {
        System.out.print(((JsonObject)part).get("response").getAsString());
        System.out.flush();
      }
    }
  }
}

/*
 * Local Variables:
 * compile-command: "javac -d . -cp /usr/share/java/gson.jar OllamaSpit.java"
 * run-command: "echo 'Translate into German: "Welcome to the GNU project!"' | java -cp .:/usr/share/java/gson.jar OllamaSpit --model=ministral-3:14b"
 * End:
 */

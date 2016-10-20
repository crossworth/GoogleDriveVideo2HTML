/*
Copyright (c) 2016 - Pedro Henrique <system.pedrohenrique@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
This software uses Browser from Patrick Louis -> https://github.com/venam/Browser
Please check the License on the Browser.hpp file.
*/

#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <sstream>

#include "Browser/Browser.hpp"


const std::regex TEST_GOOGLE_LINK("https:\\/\\/(drive|docs)\\.google\\.com\\/(file\\/d\\/|uc\?id=)([a-zA-Z0-9-]+)");
const std::regex TEST_GOOGLE_LINK_REAL("\\/[^\\/]+\\.google\\.com");
const std::string GOOGLE_REDIRECTOR("/redirector.googlevideo.com");


void print_usage() {
	std::cout << "GoogleDrive2HTML5 - PedroHenrique.ninja" << std::endl;
	std::cout << "Usage: drive2html LINK [JSON]" << std::endl;
	std::cout << "\tLINK - Google Drive link" << std::endl;
	std::cout << "\tJSON - Output to json format" << std::endl << std::endl;
}

std::vector<std::string> explode(const std::string& string, const char& delimiter) {
    std::vector<std::string> result;
    std::istringstream iss(string);

    for (std::string token; std::getline(iss, token, delimiter);) {
        result.push_back(move(token));
    }

    return result;
}

std::string string_replace(const std::string& source, const std::string& search,
						   const std::string& replace) {
	std::string result = source;
	size_t pos = std::string::npos;
	while ((pos = result.find(search)) != std::string::npos) {
		result.replace(pos, search.length(), replace);
	}
	return result;
}

std::string get_google_drive_id(const std::string& url) {
	std::smatch result;
	if (std::regex_search(url, result, TEST_GOOGLE_LINK)) {
		if (result.size() == 4) {
			return std::string(result[3]);
		}
	}

	return std::string("");
}

std::string get_real_link(const std::string& link) {
	return std::regex_replace(link, TEST_GOOGLE_LINK_REAL, GOOGLE_REDIRECTOR);
}

void process_google_drive_id(const std::string& id, bool print_to_json = false) {
	std::string base_link = "https://drive.google.com/file/d/" + id + "/view?pli=1";

	Browser br;
	br.set_handle_redirect(true);
	br.open(base_link);

	 do {
        br.open(base_link);
    } while(!br.viewing_html() || br.status() != "200" || br.error());

	std::string response = br.response();
	br.close();

	std::string start_pattern = "[\"fmt_stream_map\",\"";
	std::string end_pattern = "\"]";

	auto pos = response.find(start_pattern);

	if (pos != std::string::npos) {
		std::string result_string = response.substr(pos + start_pattern.size());
		pos = result_string.find_first_of(end_pattern);
		result_string = result_string.substr(0, pos);
		result_string = string_replace(result_string, std::string("\\u003d"), std::string("="));
		result_string = string_replace(result_string, std::string("\\u0026"), std::string("&"));

		std::vector<std::string> results = explode(result_string, ',');

		if (print_to_json) {
			std::cout << "[";
		}

		int size = results.size();
		for (auto& r : results) {
			std::vector<std::string> prefix = explode(r, '|');
			int type = std::stoi(prefix[0]);
			std::string link = prefix[1];
			std::string quality;

			switch (type) {
				case 5: { quality = "240p h263 flv"; } break;
				case 18: { quality = "360p h264 mp4"; } break;
				case 22: { quality = "720p h264 mp4"; } break;
				case 34: { quality = "360p h264 flv"; } break;
				case 35: { quality = "480p h264 flv"; } break;
				case 37: { quality = "1080p h264 mp4"; } break;
				case 36: { quality = "3gpp"; } break;
				case 38: { quality = "720p vp8 webm"; } break;
				case 43: { quality = "360p h264 flv"; } break;
				case 44: { quality = "480p vp8 webm"; } break;
				case 45: { quality = "720p vp8 webm"; } break;
				case 46: { quality = "520p vp8 webm"; } break;
				case 59: { quality = "480 for rtmpe"; } break;
				case 78: { quality = "400 for rtmpe"; } break;
				case 82: { quality = "360p h264 stereo"; } break;
				case 83: { quality = "240p h264 stereo"; } break;
				case 84: { quality = "720p h264 stereo"; } break;
				case 85: { quality = "520p h264 stereo"; } break;
				case 100: { quality = "360p vp8 webm stereo"; } break;
				case 101: { quality = "480p vp8 webm stereo"; } break;
				case 102: { quality = "720p vp8 webm stereo"; } break;
				case 120: { quality = "hd720"; } break;
				case 121: { quality = "hd1080"; } break;
				default: { quality = "unknow"; } break;
			}

			std::string real_video_link = get_real_link(link);
			--size;
			if (print_to_json) {
				std::cout << "{";
				std::cout << "\"type\":";
				std::cout << "\"";
				std::cout << type;
				std::cout << "\", ";
				std::cout << "\"quality\":";
				std::cout << "\"";
				std::cout << quality;
				std::cout << "\", ";
				std::cout << "\"link\":";
				std::cout << "\"";
				std::cout << real_video_link;
				std::cout << "\"";
				if (size > 0) {
					std::cout << "}, ";
				} else {
					std::cout << "}";
				}
			} else {
				std::cout << type << " " << quality << " " << real_video_link << std::endl;
			}
		}

		if (print_to_json) {
			std::cout << "]";
		}

	} else {
		std::cout << "Error while parsing GoogleDriver response" << std::endl;
		return;
	}
}

int main(int argc, char** argv) {

	// program_name google_drive_link [output json]
	if (argc < 2) {
		print_usage();
		return 0;
	}

	std::string google_drive_link = std::string(argv[1]);
	std::string google_drive_video_id = get_google_drive_id(google_drive_link);


	bool output_json = false;

	if (argc == 3) {
		output_json = true;
	}

	if (google_drive_video_id == "") {
		std::cout << "Invalid Google Drive URL" << std::endl;
		return 0;
	}

	process_google_drive_id(google_drive_video_id, output_json);

	return 0;
}

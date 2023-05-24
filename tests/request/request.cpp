#include <curl/curl.h>
#include <iostream>
#include <string>

size_t write_data(char* ptr, size_t size, size_t nmemb, std::string* data) {
    if (data == nullptr) {
        return 0;
    }
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

int main() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Error initializing libcurl" << std::endl;
        return 1;
    }

    std::string url = "http://127.0.0.1:50882";
    std::string payload = R"({
        "id": "5135958352",
        "jsonrpc": "2.0",
        "method": "query",
        "params": {
            "sql": "SELECT * FROM categories WHERE name = 'Food & Beverages';"
        }
    })";//"{\"id\": \"5135958352\", \"jsonrpc\": \"2.0\", \"method\": \"query\", \"params\": {\"sql\": \"SELECT * FROM users WHERE name = 'layter'\"}}";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Content-Length: " + std::to_string(payload.length())).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // set timeout to 10 seconds
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    std::string response_data;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Error sending HTTP request: " << curl_easy_strerror(res) << std::endl;
    } else {
        std::cout << response_data << std::endl;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return 0;
} // g++ request.cpp -o request -lcurl


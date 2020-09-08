========================================
curlpool—fast async HTTP requests in C++
========================================

What is this?
-------------

curlpool implements asynchronous HTTP requests (via libcurl) running
on an event loop (via libuv), scaling to many thousands of requests
per second on a single thread.

This was created out of a need to do async HTTP requests in an async
server, similar to how it's done in nodejs, without blocking the an
entire thread for the duration of a request.

Usage
-----

::

  #include "curlpool.h"
  int main(int argc, char* argv[]) {
    curlpool::cURLPool pool{};
    for (int i = 0; i < 100; ++i) {
      auto* curl = curl_easy_init();
  	  curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");
  	  // etc... create a cURL easy handle as usual
  	  // but don't call `curl_easy_perform(curl)`! do this instead:
  	  pool.add_handle(curl);
    }
    pool.run();
  }
  

License
-------

Apache-2.0

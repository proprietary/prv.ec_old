======
Design
======

Requirements
------------

- Barebones URL shortening web service
- No logging
- Option to encrypt URLs with a user-supplied key like Mega


Architecture
------------

C++ server:

#. main I/O loop managed by libuv
#. RocksDB database (on EFS? with S3 backups?)
#. server-side encryption of URLs (opt-in, for noscript fallback): openssl/boringssl
 
Frontend:

#. simple HTML5 form
#. JS: client side crypto
#. noscript fallback

Crypto
------

Preferentially, key derivation [#deriv]_ and
encryption [#encrypt]_ is done client-side in the browser. For
noscript browsers, server-side encryption serves as a fallback. This
limits the choice of algorithms to those also implemented in the browser.

Key derivation:

- PBKDF2
- ECDH

Encryption:

- AES-GCM
- AES-CTR

  
.. [#deriv] https://developer.mozilla.org/en-US/docs/Web/API/SubtleCrypto/deriveKey

.. [#encrypt] https://developer.mozilla.org/en-US/docs/Web/API/SubtleCrypto/encrypt


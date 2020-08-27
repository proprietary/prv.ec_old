==============================
prv.ec — private URL shortener
==============================

The world's first URL shortener that doesn't need to know the URL

Required environment variables
------------------------------

- EC_PRV_HCAPTCHA_SITEKEY — API token from hcaptcha.com
- EC_PRV_HCAPTCHA_SECRET — API token from hcaptcha.com
- EC_PRV_ROCKSDB_DATADIR_PATH — absolute path to a directory which will hold the embedded database
- EC_PRV_RPC_USER — username to access "trusted" endpoints with HTTP Basic Auth (this feature will be removed)
- EC_PRV_RPC_PASS — password to access "trusted" endpoints with HTTP Basic Auth (this feature will be removed)

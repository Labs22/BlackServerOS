import hashpumpy
import hashlib

key           = b"KEY"*4
original_data = b"ORIG"*4
data_to_add   = b"system('/bin/sh')"

for algorithm in (hashlib.md5, hashlib.sha1, hashlib.sha256, hashlib.sha512):
    original_digest      = algorithm(key + original_data).hexdigest()
    new_digest, new_data = hashpumpy.hashpump(original_digest, original_data, data_to_add, len(key))
    verify_digest        = algorithm(key + new_data).hexdigest()

    assert new_digest == verify_digest

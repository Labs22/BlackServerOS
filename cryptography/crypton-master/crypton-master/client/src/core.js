/* Crypton Client, Copyright 2013, 2014, 2015 SpiderOak, Inc.
 *
 * This file is part of Crypton Client.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

var crypton = {};

(function() {

  'use strict';

  var MISMATCH_ERR = 'Server and client version mismatch';

  /**!
   * ### scriptName
   * Holds the URL of the script that contains this code.
   * This will be used by the 'isomerize' function and can be
   * removed if it is removed.
   */
  if (document && document.getElementsByTagName) {
    var scripts = document.getElementsByTagName('script');
    var script = scripts[scripts.length - 1].src;
    crypton.scriptName = script;
  }

  /**!
   * ### version
   * Holds framework version for potential future backward compatibility.
   * 'PACKAGE_VERSION' string is replaced with the version from package.json
   * at build time
   */
  crypton.version = 'PACKAGE_VERSION';

  /**!
   * ### MIN_PBKDF2_ROUNDS
   * Minimum number of PBKDF2 rounds
   */
  crypton.MIN_PBKDF2_ROUNDS = 1000;

  /**!
   * ### clientVersionMismatch
   * Holds cleint <-> server version mismatch status
   */
  crypton.clientVersionMismatch = undefined;

  crypton.versionCheck = function(skip, callback) {
    if (skip) {
      return callback(null);
    }

    var url = crypton.url() + '/versioncheck?' + 'v=' + crypton.version + '&sid=' + crypton.sessionId || '';
    superagent.get(url)
      .end(function(res) {

        if (res.body.success !== true && res.body.error !== undefined) {
          crypton.clientVersionMismatch = true;
          return callback(res.body.error);
        }

        callback(null);
      });
  };

  /**!
   * ### host
   * Holds location of Crypton server
   */
  crypton.host = location.hostname;

  /**!
   * ### port
   * Holds port of Crypton server
   */
  crypton.port = 1025;

  /**!
   * ### cipherOptions
   * Sets AES mode to GCM, necessary for SJCL
   */
  crypton.cipherOptions = {
    mode: 'gcm',
  };

  /**!
   * ### paranoia
   * Tells SJCL how strict to be about PRNG readiness
   */
  crypton.paranoia = 6;

  /**!
   * ### trustedPeers
   * Internal name for trusted peer (contacts list)
   */
  crypton.trustedPeers = '_trusted_peers';

  /**!
   * ### collectorsStarted
   * Internal flag to know if startCollectors has been called
   */
  crypton.collectorsStarted = false;

  /**!
   * ### startCollectors
   * Start sjcl.random listeners for adding to entropy pool
   */
  crypton.startCollectors = function() {
    sjcl.random.startCollectors();
    crypton.collectorsStarted = true;
  };

  /**!
   * ### url()
   * Generate URLs for server calls
   *
   * @return {String} url
   */
  crypton.url = function() {
    return 'https://' + crypton.host + ':' + crypton.port;
  };

  /**!
   * ### randomBytes(nbytes)
   * Generate `nbytes` bytes of random data
   *
   * @param {Number} nbytes
   * @return {Array} bitArray
   */
  function randomBytes(nbytes) {
    if (!nbytes) {
      throw new Error('randomBytes requires input');
    }

    if (parseInt(nbytes, 10) !== nbytes) {
      throw new Error('randomBytes requires integer input');
    }

    if (nbytes < 4) {
      throw new Error('randomBytes cannot return less than 4 bytes');
    }

    if (nbytes % 4 !== 0) {
      throw new Error('randomBytes requires input as multiple of 4');
    }

    // sjcl's words are 4 bytes (32 bits)
    var nwords = nbytes / 4;
    return sjcl.random.randomWords(nwords);
  }

  crypton.randomBytes = randomBytes;

  /**!
   * ### constEqual()
   * Compare two strings in constant time.
   *
   * @param {String} str1
   * @param {String} str2
   * @return {bool} equal
   */
  function constEqual(str1, str2) {
    // We only support string comparison, we could support Arrays but
    // they would need to be single char elements or compare multichar
    // elements constantly. Going for simplicity for now.
    // TODO: Consider this ^
    if (typeof str1 !== 'string' || typeof str2 !== 'string') {
      return false;
    }

    var mismatch = str1.length ^ str2.length;
    var len = Math.min(str1.length, str2.length);

    for (var i = 0; i < len; i++) {
      mismatch |= str1.charCodeAt(i) ^ str2.charCodeAt(i);
    }

    return mismatch === 0;
  }

  crypton.constEqual = constEqual;
  crypton.sessionId = null;

  /**!
   * ### randomBits(nbits)
   * Generate `nbits` bits of random data
   *
   * @param {Number} nbits
   * @return {Array} bitArray
   */
  crypton.randomBits = function(nbits) {
    if (!nbits) {
      throw new Error('randomBits requires input');
    }

    if (parseInt(nbits, 10) !== nbits) {
      throw new Error('randomBits requires integer input');
    }

    if (nbits < 32) {
      throw new Error('randomBits cannot return less than 32 bits');
    }

    if (nbits % 32 !== 0) {
      throw new Error('randomBits requires input as multiple of 32');
    }

    var nbytes = nbits / 8;
    return crypton.randomBytes(nbytes);
  };

  /**!
   * ### mac(key, data)
   * Generate an HMAC using `key` for `data`.
   *
   * @param {String} key
   * @param {String} data
   * @return {String} hmacHex
   */
  crypton.hmac = function(key, data) {
    var mac = new sjcl.misc.hmac(key);
    return sjcl.codec.hex.fromBits(mac.mac(data));
  };

  /**!
   * ### macAndCompare(key, data, otherMac)
   * Generate an HMAC using `key` for `data` and compare it in
   * constant time to `otherMac`.
   *
   * @param {String} key
   * @param {String} data
   * @param {String} otherMac
   * @return {Bool} compare succeeded
   */
  crypton.hmacAndCompare = function(key, data, otherMac) {
    var ourMac = crypton.hmac(key, data);
    return crypton.constEqual(ourMac, otherMac);
  };

  /**!
   * ### fingerprint(pubKey, signKeyPub)
   * Generate a fingerprint for an account or peer.
   *
   * @param {PublicKey} pubKey
   * @param {PublicKey} signKeyPub
   * @return {String} hash
   */
  crypton.fingerprint = function(pubKey, signKeyPub) {
    // TODO check inputs
    var pubKeys = sjcl.bitArray.concat(
      pubKey._point.toBits(),
      signKeyPub._point.toBits()
    );

    return crypton.hmac('', pubKeys);
  };

  /**!
   * ### generateAccount(username, passphrase, callback, options)
   * Generate salts and keys necessary for an account
   *
   * Saves account to server unless `options.save` is falsey
   *
   * Calls back with account and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} username
   * @param {String} passphrase
   * @param {Function} callback
   * @param {Object} options
   */
  crypton.generateAccount = function(username, passphrase, callback, options) {
    // TODO consider moving non-callback arguments to single object
    if (crypton.clientVersionMismatch) {
      return callback(MISMATCH_ERR);
    }

    options = options || {};
    var save = typeof options.save !== 'undefined' ? options.save : true;

    crypton.versionCheck(!save, function(err) {
      if (err) {
        return callback(MISMATCH_ERR);
      } else {

        if (!username || !passphrase) {
          return callback('Must supply username and passphrase');
        }

        if (!crypton.collectorsStarted) {
          crypton.startCollectors();
        }

        var SIGN_KEY_BIT_LENGTH = 384;
        var keypairCurve = options.keypairCurve || 384;
        var numRounds = crypton.MIN_PBKDF2_ROUNDS;

        var account = new crypton.Account();
        var hmacKey = randomBytes(32);
        var keypairSalt = randomBytes(32);
        var keypairMacSalt = randomBytes(32);
        var signKeyPrivateMacSalt = randomBytes(32);
        var containerNameHmacKey = randomBytes(32);
        var keypairKey = sjcl.misc.pbkdf2(passphrase, keypairSalt, numRounds);
        var keypairMacKey = sjcl.misc.pbkdf2(passphrase, keypairMacSalt, numRounds);
        var signKeyPrivateMacKey = sjcl.misc.pbkdf2(passphrase, signKeyPrivateMacSalt, numRounds);
        var keypair = sjcl.ecc.elGamal.generateKeys(keypairCurve, crypton.paranoia);
        var signingKeys = sjcl.ecc.ecdsa.generateKeys(SIGN_KEY_BIT_LENGTH, crypton.paranoia);
        var srp = new SRPClient(username, passphrase, 2048, 'sha-256');
        var srpSalt = srp.randomHexSalt();
        var srpVerifier = srp.calculateV(srpSalt).toString(16);

        account.username = username;
        account.keypairSalt = JSON.stringify(keypairSalt);
        account.keypairMacSalt = JSON.stringify(keypairMacSalt);
        account.signKeyPrivateMacSalt = JSON.stringify(signKeyPrivateMacSalt);

        // Pad verifier to 512 bytes
        // TODO: This length will change when a different SRP group is used
        account.srpVerifier = srp.nZeros(512 - srpVerifier.length) + srpVerifier;
        account.srpSalt = srpSalt;

        // pubkeys
        account.pubKey = JSON.stringify(keypair.pub.serialize());
        account.signKeyPub = JSON.stringify(signingKeys.pub.serialize());

        var sessionIdentifier = 'dummySession';
        var session = new crypton.Session(sessionIdentifier);
        session.account = account;
        session.account.signKeyPrivate = signingKeys.sec;

        var selfPeer = new crypton.Peer({
          session: session,
          pubKey: keypair.pub,
          signKeyPub: signingKeys.pub,
        });
        selfPeer.trusted = true;

        // hmac keys
        var encryptedHmacKey = selfPeer.encryptAndSign(JSON.stringify(hmacKey));
        if (encryptedHmacKey.error) {
          callback(encryptedHmacKey.error, null);
          return;
        }

        account.hmacKeyCiphertext = JSON.stringify(encryptedHmacKey);

        var encryptedContainerNameHmacKey = selfPeer.encryptAndSign(JSON.stringify(containerNameHmacKey));
        if (encryptedContainerNameHmacKey.error) {
          callback(encryptedContainerNameHmacKey.error, null);
          return;
        }

        account.containerNameHmacKeyCiphertext = JSON.stringify(encryptedContainerNameHmacKey);

        // private keys
        // TODO: Check data auth with hmac
        var keypairCiphertext = sjcl.encrypt(keypairKey, JSON.stringify(keypair.sec.serialize()), crypton.cipherOptions);

        account.keypairCiphertext = keypairCiphertext;
        account.keypairMac = crypton.hmac(keypairMacKey, account.keypairCiphertext);
        account.signKeyPrivateCiphertext = sjcl.encrypt(keypairKey, JSON.stringify(signingKeys.sec.serialize()), crypton.cipherOptions);
        account.signKeyPrivateMac = crypton.hmac(signKeyPrivateMacKey, account.signKeyPrivateCiphertext);

        if (save) {
          account.save(function(err) {
            callback(err, account);
          });

          return;
        }

        callback(null, account);
      }
    });
  };

  /**!
   * ### authorize(username, passphrase, callback)
   * Perform zero-knowledge authorization with given `username`
   * and `passphrase`, generating a session if successful
   *
   * Calls back with session and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * SRP variables are named as defined in RFC 5054
   * and RFC 2945, prefixed with 'srp'
   *
   * @param {String} username
   * @param {String} passphrase
   * @param {Function} callback
   * @param {Object} options
   */
  crypton.authorize = function(username, passphrase, callback, options) {
    if (crypton.clientVersionMismatch) {
      return callback(MISMATCH_ERR);
    }

    options = options || {};
    var check = typeof options.check !== 'undefined' ? options.check : true;

    crypton.versionCheck(!check, function(err) {
      if (err) {
        return callback(MISMATCH_ERR);
      } else {

        if (!username || !passphrase) {
          return callback('Must supply username and passphrase');
        }

        if (!crypton.collectorsStarted) {
          crypton.startCollectors();
        }

        var options = {
          username: username,
          passphrase: passphrase,
        };

        crypton.work.calculateSrpA(options, function(err, data) {
          if (err) {
            return callback(err);
          }

          var response = {
            srpA: data.srpAstr,
          };

          superagent.post(crypton.url() + '/account/' + username)
            .withCredentials()
            .send(response)
            .end(function(res) {
              if (!res.body || res.body.success !== true) {
                return callback(res.body.error);
              }

              // check for response session header:
              // XXX: Make sure we have a sid!
              crypton.sessionId = res.body.sid;
              window.sessionStorage.setItem('sessionId', res.body.sid);

              options.a = data.a;
              options.srpA = data.srpA;
              options.srpB = res.body.srpB;
              options.srpSalt = res.body.srpSalt;

              // calculateSrpM1
              crypton.work.calculateSrpM1(options, function(err, srpM1, ourSrpM2) {
                response = {
                  srpM1: srpM1,
                };

                var url = crypton.url() +
                  '/account/' + username + '/answer?sid=' + crypton.sessionId;
                superagent.post(url)
                  .withCredentials()
                  .send(response)
                  .end(function(res) {
                    if (!res.body || res.body.success !== true) {
                      callback(res.body.error);
                      return;
                    }

                    if (!constEqual(res.body.srpM2, ourSrpM2)) {
                      callback('Server could not be verified');
                      return;
                    }

                    var session = new crypton.Session(crypton.sessionId);
                    session.account = new crypton.Account();
                    session.account.username = username;
                    session.account.passphrase = passphrase;
                    session.account.challengeKey = res.body.account.challengeKey;
                    session.account.containerNameHmacKeyCiphertext = res.body.account.containerNameHmacKeyCiphertext;
                    session.account.hmacKeyCiphertext = res.body.account.hmacKeyCiphertext;
                    session.account.keypairCiphertext = res.body.account.keypairCiphertext;
                    session.account.keypairMac = res.body.account.keypairMac;
                    session.account.pubKey = res.body.account.pubKey;
                    session.account.challengeKeySalt = res.body.account.challengeKeySalt;
                    session.account.keypairSalt = res.body.account.keypairSalt;
                    session.account.keypairMacSalt = res.body.account.keypairMacSalt;
                    session.account.signKeyPub = res.body.account.signKeyPub;
                    session.account.signKeyPrivateCiphertext = res.body.account.signKeyPrivateCiphertext;
                    session.account.signKeyPrivateMacSalt = res.body.account.signKeyPrivateMacSalt;
                    session.account.signKeyPrivateMac = res.body.account.signKeyPrivateMac;
                    session.account.unravel(function(err) {
                      if (err) {
                        return callback(err);
                      }

                      // check for internal 'trusted peers' Item
                      session.getOrCreateItem(crypton.trustedPeers,
                        function(err, item) {
                          if (err) {
                            var _err = 'Cannot get "trusted peers" Item';
                            console.error(_err, err);

                            // still need to return the session
                            return callback(_err, session);
                          }

                          return callback(null, session);
                        });
                    });
                  });
              });
            });
        });
      }
    });
  };
})();

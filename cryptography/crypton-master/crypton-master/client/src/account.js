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

(function() {

  'use strict';

  /**!
   * # Account()
   *
   * ````
   * var account = new crypton.Account();
   * ````
   */
  var Account = crypton.Account = function Account() {};

  /**!
   * ### save(callback)
   * Send the current account to the server to be saved
   *
   * Calls back without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Function} callback
   */
  Account.prototype.save = function(callback) {
    superagent.post(crypton.url() + '/account?sid=' + crypton.sessionId)
      .withCredentials()
      .send(this.serialize())
      .end(function(res) {
        if (res.body.success !== true) {
          callback(res.body.error);
        } else {
          callback();
        }
      });
  };

  /**!
   * ### unravel(callback)
   * Decrypt raw account object from server after successful authentication
   *
   * Calls back without error if successful
   *
   * __Throws__ if unsuccessful
   *
   * @param {Function} callback
   */
  Account.prototype.unravel = function(callback) {
    var _this = this;
    crypton.work.unravelAccount(this, function(err, data) {
      if (err) {
        return callback(err);
      }

      _this.regenerateKeys(data, function(err) {
        callback(err);
      });
    });
  };

  /**!
   * ### regenerateKeys(callback)
   * Reconstruct keys from unraveled data
   *
   * Calls back without error if successful
   *
   * __Throws__ if unsuccessful
   *
   * @param {Function} callback
   */
  Account.prototype.regenerateKeys = function(data, callback) {
    // reconstruct secret key
    var exponent = sjcl.bn.fromBits(data.secret.exponent);
    this.secretKey = new sjcl.ecc.elGamal.secretKey(data.secret.curve, sjcl.ecc.curves['c' + data.secret.curve], exponent);

    // reconstruct public key
    var point = sjcl.ecc.curves['c' + this.pubKey.curve].fromBits(this.pubKey.point);
    this.pubKey = new sjcl.ecc.elGamal.publicKey(this.pubKey.curve, point.curve, point);

    // assign the hmac keys to the account
    this.hmacKey = data.hmacKey;
    this.containerNameHmacKey = data.containerNameHmacKey;

    // reconstruct the public signing key
    var signPoint = sjcl.ecc.curves['c' + this.signKeyPub.curve].fromBits(this.signKeyPub.point);
    this.signKeyPub = new sjcl.ecc.ecdsa.publicKey(this.signKeyPub.curve, signPoint.curve, signPoint);

    // reconstruct the secret signing key
    var signExponent = sjcl.bn.fromBits(data.signKeySecret.exponent);
    this.signKeyPrivate = new sjcl.ecc.ecdsa.secretKey(data.signKeySecret.curve, sjcl.ecc.curves['c' + data.signKeySecret.curve], signExponent);

    // calculate fingerprint for public key
    this.fingerprint = crypton.fingerprint(this.pubKey, this.signKeyPub);

    // recalculate the public points from secret exponents
    // and verify that they match what the server sent us
    var pubKeyHex = sjcl.codec.hex.fromBits(this.pubKey._point.toBits());
    var pubKeyShouldBe = this.secretKey._curve.G.mult(exponent);
    var pubKeyShouldBeHex = sjcl.codec.hex.fromBits(pubKeyShouldBe.toBits());

    if (!crypton.constEqual(pubKeyHex, pubKeyShouldBeHex)) {
      return callback('Server provided incorrect public key');
    }

    var signKeyPubHex = sjcl.codec.hex.fromBits(this.signKeyPub._point.toBits());
    var signKeyPubShouldBe = this.signKeyPrivate._curve.G.mult(signExponent);
    var signKeyPubShouldBeHex = sjcl.codec.hex.fromBits(signKeyPubShouldBe.toBits());

    if (!crypton.constEqual(signKeyPubHex, signKeyPubShouldBeHex)) {
      return callback('Server provided incorrect public signing key');
    }

    // sometimes the account object is used as a peer
    // to make the code simpler. verifyAndDecrypt checks
    // that the peer it is passed is trusted, or returns
    // an error. if we've gotten this far, we can be sure
    // that the public keys are trustable.
    this.trusted = true;

    callback(null);
  };

  // TODO rename to toJSON
  /**!
   * ### serialize()
   * Package and return a JSON representation of the current account
   *
   * @return {Object}
   */
  Account.prototype.serialize = function() {
    return {
      srpVerifier: this.srpVerifier,
      srpSalt: this.srpSalt,
      containerNameHmacKeyCiphertext: this.containerNameHmacKeyCiphertext,
      hmacKeyCiphertext: this.hmacKeyCiphertext,
      keypairCiphertext: this.keypairCiphertext,
      keypairMac: this.keypairMac,
      pubKey: this.pubKey,
      keypairSalt: this.keypairSalt,
      keypairMacSalt: this.keypairMacSalt,
      signKeyPrivateMacSalt: this.signKeyPrivateMacSalt,
      username: this.username,
      signKeyPub: this.signKeyPub,
      signKeyPrivateCiphertext: this.signKeyPrivateCiphertext,
      signKeyPrivateMac: this.signKeyPrivateMac,
    };
  };

  /**!
   * ### verifyAndDecrypt()
   * Convienence function to verify and decrypt public key encrypted & signed data
   *
   * @return {Object}
   */
  Account.prototype.verifyAndDecrypt = function(signedCiphertext, peer) {
    if (!peer.trusted) {
      return {
        error: 'Peer is untrusted',
      };
    }

    // hash the ciphertext
    var ciphertextString = JSON.stringify(signedCiphertext.ciphertext);
    var hash = sjcl.hash.sha256.hash(ciphertextString);

    // verify the signature
    var verified = false;
    try {
      verified = peer.signKeyPub.verify(hash, signedCiphertext.signature);
    } catch (ex) {
      console.error(ex);
      console.error(ex.stack);
    }

    // try to decrypt regardless of verification failure
    try {
      var message = sjcl.decrypt(this.secretKey, ciphertextString, crypton.cipherOptions);
      if (verified) {
        return {
          plaintext: message,
          verified: verified,
          error: null,
        };
      } else {
        return {
          plaintext: null,
          verified: false,
          error: 'Cannot verify ciphertext',
        };
      }
    } catch (ex) {
      console.error(ex);
      console.error(ex.stack);
      return {
        plaintext: null,
        verified: false,
        error: 'Cannot verify ciphertext',
      };
    }
  };

  /**!
   * ### changePassphrase()
   * Convienence function to change the user's passphrase
   *
   * @param {String} currentPassphrase
   * @param {String} newPassphrase
   * @param {Function} callback
   * callback will be handed arguments err, isComplete
   * Upon completion of a passphrase change, the client will be logged out
   * This callback should handle getting the user logged back in
   * programmatically or via the UI
   * @param {Function} keygenProgressCallback [optional]
   * @param {Boolean} skipCheck [optional]
   * @return void
   */
  Account.prototype.changePassphrase =
    function(currentPassphrase, newPassphrase,
      callback, keygenProgressCallback, skipCheck) {
      if (skipCheck) {
        if (currentPassphrase === newPassphrase) {
          var err = 'New passphrase cannot be the same as current password';
          return callback(err);
        }
      }

      if (keygenProgressCallback) {
        if (typeof keygenProgressCallback === 'function') {
          keygenProgressCallback();
        }
      }

      var MIN_PBKDF2_ROUNDS = crypton.MIN_PBKDF2_ROUNDS;
      var _this = this;
      var username = this.username;

      // authorize to make sure the user knows the correct passphrase
      crypton.authorize(username, currentPassphrase, function(err, newSession) {
        if (err) {
          console.error(err);
          return callback(err);
        }

        // We have authorized, time to create the new keyring parts we
        // need to update the database

        var currentAccount = newSession.account;

        // Replace all salts with new ones
        var keypairSalt = crypton.randomBytes(32);
        var keypairMacSalt = crypton.randomBytes(32);
        var signKeyPrivateMacSalt = crypton.randomBytes(32);

        var srp = new SRPClient(username, newPassphrase, 2048, 'sha-256');
        var srpSalt = srp.randomHexSalt();
        var srpVerifier = srp.calculateV(srpSalt).toString(16);

        // Pad verifier to 512 bytes
        // TODO: This length will change when a different SRP group is used
        srpVerifier = srp.nZeros(512 - srpVerifier.length) + srpVerifier;

        var keypairKey =
          sjcl.misc.pbkdf2(newPassphrase, keypairSalt, MIN_PBKDF2_ROUNDS);

        var keypairMacKey =
          sjcl.misc.pbkdf2(newPassphrase, keypairMacSalt, MIN_PBKDF2_ROUNDS);

        var signKeyPrivateMacKey =
          sjcl.misc.pbkdf2(newPassphrase, signKeyPrivateMacSalt, MIN_PBKDF2_ROUNDS);

        var privateKeys = {
          // 'privateKey/HMAC result name': serializedKey or string HMAC input data
          containerNameHmacKeyCiphertext: currentAccount.containerNameHmacKey,
          hmacKeyCiphertext: currentAccount.hmacKey,
          signKeyPrivateCiphertext: currentAccount.signKeyPrivate.serialize(),
          keypairCiphertext: currentAccount.secretKey.serialize(),
          keypairMacKey: keypairMacKey,
          signKeyPrivateMacKey: signKeyPrivateMacKey,
        };

        var newKeyring;

        try {
          newKeyring = _this.wrapAllKeys(keypairKey, privateKeys, newSession);
        } catch (ex) {
          console.error(ex);
          console.error(ex.stack);
          return callback('Fatal: cannot wrap keys, see error console for more information');
        }

        // Set other new properties before we save
        newKeyring.keypairSalt = JSON.stringify(keypairSalt);
        newKeyring.keypairMacSalt = JSON.stringify(keypairMacSalt);
        newKeyring.signKeyPrivateMacSalt = JSON.stringify(signKeyPrivateMacSalt);
        newKeyring.srpVerifier = srpVerifier;
        newKeyring.srpSalt = srpSalt;
        var url = crypton.url() +
          '/account/' +
          _this.username +
          '/keyring?sid=' +
          crypton.sessionId;

        superagent.post(url)
          .withCredentials()
          .send(newKeyring)
          .end(function(res) {
            if (res.body.success !== true) {
              console.error('error: ', res.body.error);
              callback(res.body.error);
            } else {
              // XXX TODO: Invalidate all other client sessions before doing:
              newSession = null; // Force new login after passphrase change
              callback(null, true); // Do not hand the new session to the callback
            }
          });

      }, null);
    };

  /**!
   * ### wrapKey()
   * Helper function to wrap keys
   *
   * @param {String} selfPeer
   * @param {String} serializedPrivateKey
   * @return {Object} wrappedKey
   */
  Account.prototype.wrapKey = function(selfPeer, serializedPrivateKey) {
    if (!selfPeer || !serializedPrivateKey) {
      throw new Error('selfPeer and serializedPrivateKey are required');
    }

    var serializedKey;
    if (typeof serializedPrivateKey !== 'string') {
      serializedKey = JSON.stringify(serializedPrivateKey);
    } else {
      serializedKey = serializedPrivateKey;
    }

    var wrappedKey = selfPeer.encryptAndSign(serializedKey);
    if (wrappedKey.error) {
      return null;
    }

    return wrappedKey;
  };

  /**!
   * ### wrapAllKeys()
   * Helper function to wrap all keys when changing passphrase, etc
   *
   * @param {String} wrappingKey
   * @param {Object} privateKeys
   * @param {Object} Session
   * @return {Object} wrappedKey
   */
  Account.prototype.wrapAllKeys = function(wrappingKey, privateKeys, session) {
    // Using the *labels* of the future wrapped objects here
    var requiredKeys = [
      'containerNameHmacKeyCiphertext',
      'hmacKeyCiphertext',
      'signKeyPrivateCiphertext',
      'keypairCiphertext', // main encryption private key
      'keypairMacKey',
      'signKeyPrivateMacKey',
    ];

    var privateKeysLength = Object.keys(privateKeys).length;
    var privateKeyNames = Object.keys(privateKeys);

    for (var i = 0; i < privateKeysLength; i++) {
      var keyName = privateKeyNames[i];
      if (requiredKeys.indexOf(keyName) === -1) {
        throw new Error('Missing private key: ' + keyName);
      }
    }

    // Check that the length of privateKeys is correct
    if (privateKeysLength !== requiredKeys.length) {
      throw new Error('privateKeys length does not match requiredKeys length');
    }

    var selfPeer = new crypton.Peer({
      session: session,
      pubKey: session.account.pubKey,
      signKeyPub: session.account.signKeyPub,
    });
    selfPeer.trusted = true;

    var result = {};

    var hmacKeyCiphertext = this.wrapKey(selfPeer,
      privateKeys.hmacKeyCiphertext);
    if (hmacKeyCiphertext.error) {
      result.hmacKeyCiphertext = null;
    } else {
      result.hmacKeyCiphertext = JSON.stringify(hmacKeyCiphertext);
    }

    var containerNameHmacKeyCiphertext =
      this.wrapKey(selfPeer,
        privateKeys.containerNameHmacKeyCiphertext);

    if (containerNameHmacKeyCiphertext.error) {
      result.containerNameHmacKeyCiphertext = null;
    } else {
      result.containerNameHmacKeyCiphertext = JSON.stringify(containerNameHmacKeyCiphertext);
    }

    // Private Keys
    var keypairCiphertext =
      sjcl.encrypt(wrappingKey,
        JSON.stringify(privateKeys.keypairCiphertext),
        crypton.cipherOptions);

    if (keypairCiphertext.error) {
      console.error(keypairCiphertext.error);
      keypairCiphertext = null;
    }

    result.keypairCiphertext = keypairCiphertext;

    var signKeyPrivateCiphertext =
      sjcl.encrypt(wrappingKey, JSON.stringify(privateKeys.signKeyPrivateCiphertext),
        crypton.cipherOptions);

    if (signKeyPrivateCiphertext.error) {
      console.error(signKeyPrivateCiphertext.error);
      signKeyPrivateCiphertext = null;
    }

    result.signKeyPrivateCiphertext = signKeyPrivateCiphertext;

    // HMACs
    result.keypairMac =
      crypton.hmac(privateKeys.keypairMacKey, result.keypairCiphertext);

    result.signKeyPrivateMac = crypton.hmac(privateKeys.signKeyPrivateMacKey,
      result.signKeyPrivateCiphertext);

    for (var keyName in result) {
      if (!result[keyName]) {
        throw new Error('Fatal: ' + keyName + ' is null');
      }
    }

    return result;
  };

})();

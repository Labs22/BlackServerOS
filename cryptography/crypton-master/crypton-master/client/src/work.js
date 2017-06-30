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

  /*
   * if the browser supports web workers,
   * we "isomerize" crypton.work to transparently
   * put its methods in a worker and replace them
   * with a bridge API to said worker
   */
  !self.worker && window.addEventListener('load', function() {
    isomerize(crypton.work, crypton.scriptName);
  }, false);

  var work = crypton.work = {};

  /**!
   * ### calculateSrpA(options, callback)
   * First step of authorization
   *
   * Calls back with SRP A values and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} options
   * @param {Function} callback
   */
  work.calculateSrpA = function(options, callback) {
    // srpRandom uses sjcl.random
    // and crypton.work might be in a different
    // thread than where startCollectors was originally called
    if (!crypton.collectorsStarted) {
      crypton.startCollectors();
    }

    try {
      var srp = new SRPClient(options.username, options.passphrase, 2048, 'sha-256');
      var a = srp.srpRandom();
      var srpA = srp.calculateA(a);

      // Pad A out to 512 bytes
      // TODO: This length will change when a different SRP group is used
      var srpAstr = srpA.toString(16);
      srpAstr = srp.nZeros(512 - srpAstr.length) + srpAstr;

      callback(null, {
        a: a.toString(),
        srpA: srpA.toString(),
        srpAstr: srpAstr,
      });
    } catch (e) {
      console.error(e);
      callback(e);
    }
  };

  /**!
   * ### calculateSrpM1(options, callback)
   * Second step of authorization
   *
   * Calls back with SRP M1 value and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} options
   * @param {Function} callback
   */
  work.calculateSrpM1 = function(options, callback) {
    try {
      var srp = new SRPClient(options.username, options.passphrase, 2048, 'sha-256');
      var srpSalt = options.srpSalt;
      var a = new BigInteger(options.a);
      var srpA = new BigInteger(options.srpA);
      var srpB = new BigInteger(options.srpB, 16);

      var srpu = srp.calculateU(srpA, srpB);
      var srpS = srp.calculateS(srpB, options.srpSalt, srpu, a);
      var rawSrpM1 = srp.calculateMozillaM1(srpA, srpB, srpS);
      var srpM1 = rawSrpM1.toString(16);

      // Pad srpM1 to the full SHA-256 length
      srpM1 = srp.nZeros(64 - srpM1.length) + srpM1;

      var srpK = srp.calculateK(srpS);
      var srpM2 = srp.calculateMozillaM2(srpA, rawSrpM1, srpK).toString(16);

      callback(null, srpM1, srpM2);
    } catch (e) {
      console.error(e);
      callback(e);
    }
  };

  /**!
   * ### unravelAccount(account, callback)
   * Decrypt account keys, and pass them back
   * in a serialized form for reconstruction
   *
   * Calls back with key object and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} account
   * @param {Function} callback
   */
  work.unravelAccount = function(account, callback) {
    var ret = {};

    var numRounds = crypton.MIN_PBKDF2_ROUNDS;

    // regenerate keypair key from password
    var keypairKey = sjcl.misc.pbkdf2(account.passphrase, account.keypairSalt, numRounds);
    var keypairMacKey = sjcl.misc.pbkdf2(account.passphrase, account.keypairMacSalt, numRounds);
    var signKeyPrivateMacKey = sjcl.misc.pbkdf2(account.passphrase, account.signKeyPrivateMacSalt, numRounds);

    var macOk = false;

    // decrypt secret key
    try {
      var ciphertextString = JSON.stringify(account.keypairCiphertext);
      macOk = crypton.hmacAndCompare(keypairMacKey, ciphertextString, account.keypairMac);
      ret.secret = JSON.parse(sjcl.decrypt(keypairKey, ciphertextString, crypton.cipherOptions));
    } catch (e) {}

    if (!macOk || !ret.secret) {
      // TODO could be decryption or parse error - should we specify?
      return callback('Could not parse secret key');
    }

    macOk = false;

    // decrypt signing key
    try {
      var ciphertextString = JSON.stringify(account.signKeyPrivateCiphertext);
      macOk = crypton.hmacAndCompare(signKeyPrivateMacKey, ciphertextString, account.signKeyPrivateMac);
      ret.signKeySecret = JSON.parse(sjcl.decrypt(keypairKey, ciphertextString, crypton.cipherOptions));
    } catch (e) {}

    if (!macOk || !ret.signKeySecret) {
      return callback('Could not parse signKeySecret');
    }

    var exponent = sjcl.bn.fromBits(ret.secret.exponent);
    var secretKey = new sjcl.ecc.elGamal.secretKey(ret.secret.curve, sjcl.ecc.curves['c' + ret.secret.curve], exponent);

    account.secretKey = secretKey;

    var session = {};
    session.account = account;
    session.account.signKeyPrivate = ret.signKeySecret;

    var signPoint = sjcl.ecc.curves['c' + account.signKeyPub.curve].fromBits(account.signKeyPub.point);

    var selfPeer = new crypton.Peer({
      session: session,
      pubKey: account.pubKey,
      signKeyPub: new sjcl.ecc.ecdsa.publicKey(account.signKeyPub.curve, signPoint.curve, signPoint),
    });
    selfPeer.trusted = true;

    var selfAccount = new crypton.Account();
    selfAccount.secretKey = secretKey;

    // decrypt hmac keys
    var containerNameHmacKey;
    try {
      containerNameHmacKey = selfAccount.verifyAndDecrypt(account.containerNameHmacKeyCiphertext, selfPeer);
      ret.containerNameHmacKey = JSON.parse(containerNameHmacKey.plaintext);
    } catch (e) {}

    if (!containerNameHmacKey.verified) {
      // TODO could be decryption or parse error - should we specify?
      return callback('Could not parse containerNameHmacKey');
    }

    var hmacKey;
    try {
      hmacKey = selfAccount.verifyAndDecrypt(account.hmacKeyCiphertext, selfPeer);
      ret.hmacKey = JSON.parse(hmacKey.plaintext);
    } catch (e) {}

    if (!hmacKey.verified) {
      // TODO could be decryption or parse error - should we specify?
      return callback('Could not parse hmacKey');
    }

    callback(null, ret);
  };

  /**!
   * ### decryptRecord(options, callback)
   * Decrypt a single record after checking its signature
   *
   * Calls back with decrypted record and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} options
   * @param {Function} callback
   */
  work.decryptRecord = function(options, callback) {
    var sessionKey = options.sessionKey;
    var creationTime = options.creationTime;
    var expectedRecordIndex = options.expectedRecordIndex;
    var peerSignKeyPubSerialized = options.peerSignKeyPubSerialized;

    if (!sessionKey ||
      !creationTime ||
      !expectedRecordIndex ||
      !peerSignKeyPubSerialized
    ) {
      return callback('Must supply all options to work.decryptRecord');
    }

    var record;
    try {
      record = JSON.parse(options.record);
    } catch (e) {}

    if (!record) {
      return callback('Could not parse record');
    }

    // reconstruct the peer's public signing key
    // the key itself typically has circular references which
    // we can't pass around with JSON to/from a worker
    var curve = 'c' + peerSignKeyPubSerialized.curve;
    var signPoint = sjcl.ecc.curves[curve].fromBits(peerSignKeyPubSerialized.point);
    var peerSignKeyPub = new sjcl.ecc.ecdsa.publicKey(peerSignKeyPubSerialized.curve, signPoint.curve, signPoint);

    var verified = false;
    var payloadCiphertextHash = sjcl.hash.sha256.hash(JSON.stringify(record.ciphertext));

    try {
      verified = peerSignKeyPub.verify(payloadCiphertextHash, record.signature);
    } catch (e) {
      console.error(e);
    }

    if (!verified) {
      return callback('Record signature does not match expected signature');
    }

    var payload;
    try {
      payload = JSON.parse(sjcl.decrypt(sessionKey, record.ciphertext, crypton.cipherOptions));
    } catch (e) {}

    if (!payload) {
      return callback('Could not parse record payload');
    }

    /*
    if (payload.recordIndex !== expectedRecordIndex) {
      // TODO revisit
      // XXX ecto 3/4/14 I ran into a problem with this quite a while
      // ago where recordIndexes would never match even if they obviously
      // should. It smelled like an off-by-one or state error.
      // Now that record decryption is abstracted outside container instances,
      // we will have to do it in a different way anyway
      // (there was formerly a this.recordIndex++ here)

      // return callback('Record index mismatch');
    }
    */

    callback(null, {
      time: +new Date(creationTime),
      delta: payload.delta,
    });
  };

})();

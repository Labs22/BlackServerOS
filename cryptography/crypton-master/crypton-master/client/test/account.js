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

'use strict';

var assert = chai.assert;

// tests running inside phantom on Travis CI
// don't let the PRNG seed fast enough
crypton.paranoia = 0;
sjcl.random.addEntropy('foo', 1024);

var assert = chai.assert;

function setupAccount() {
  var account = new crypton.Account();
  account.passphrase = 'pass';
  account.username = 'user';
  account.srpVerifier = 'verifier';
  account.srpSalt = 'salt';
  account.keypairSalt = [-1601113307, -147606214, -62907260, 1664396850, 1038241656, 596952288, -1676728508, -743835030];
  account.keypairMacSalt = [-1601113307, -147606214, -62907260, 1664396850, 1038241656, 596952288, -1676728508, -743835030];
  account.signKeyPrivateMacSalt = [-1601113307, -147606214, -62907260, 1664396850, 1038241656, 596952288, -1676728508, -743835030];

  var keypairCurve = 384;

  var hmacKey = crypton.randomBytes(32);
  var containerNameHmacKey = crypton.randomBytes(32);

  var keypairKey = sjcl.misc.pbkdf2(account.passphrase, account.keypairSalt, crypton.MIN_PBKDF2_ROUNDS);
  var keypairMacKey = sjcl.misc.pbkdf2(account.passphrase, account.keypairMacSalt, crypton.MIN_PBKDF2_ROUNDS);
  var signKeyPrivateMacKey = sjcl.misc.pbkdf2(account.passphrase, account.signKeyPrivateMacSalt, crypton.MIN_PBKDF2_ROUNDS);
  var keypair = sjcl.ecc.elGamal.generateKeys(keypairCurve, crypton.paranoia);
  var signingKeys = sjcl.ecc.ecdsa.generateKeys(384, crypton.paranoia);

  account.pubKey = keypair.pub.serialize();
  account.signKeyPub = JSON.parse(JSON.stringify(signingKeys.pub.serialize()));

  var sessionIdentifier = 'dummySession';
  var session = new crypton.Session(sessionIdentifier);
  session.account = account;
  session.account.signKeyPrivate = signingKeys.sec;

  var selfPeer = new crypton.Peer({
    session: session,
    pubKey: keypair.pub,
  });
  selfPeer.trusted = true;

  // hmac keys
  var encryptedHmacKey = selfPeer.encryptAndSign(JSON.stringify(hmacKey));
  account.hmacKeyCiphertext = encryptedHmacKey;

  var encryptedContainerNameHmacKey = selfPeer.encryptAndSign(JSON.stringify(containerNameHmacKey));
  account.containerNameHmacKeyCiphertext = encryptedContainerNameHmacKey;

  account.keypairCiphertext = JSON.parse(sjcl.encrypt(keypairKey, JSON.stringify(keypair.sec.serialize()), crypton.cipherOptions));
  account.keypairMac = crypton.hmac(keypairMacKey, JSON.stringify(account.keypairCiphertext));
  account.signKeyPrivateCiphertext = JSON.parse(sjcl.encrypt(keypairKey, JSON.stringify(signingKeys.sec.serialize()), crypton.cipherOptions));
  account.signKeyPrivateMac = crypton.hmac(signKeyPrivateMacKey, JSON.stringify(account.signKeyPrivateCiphertext));
  return account;
}

describe('Account', function() {

  this.timeout(20000);

  describe('save()', function() {
    // TODO should we just test this in the integration tests?
  });

  describe('unravel()', function() {
    it('should generate the correct fields from given values', function(done) {
      var account = setupAccount();

      account.unravel(function(err) {
        var fields = [
          'srpVerifier',
          'srpSalt',
          'secretKey',
          'pubKey',
          'containerNameHmacKey',
          'hmacKey',
          'signKeyPub',
          'signKeyPrivate',
        ];

        assert.equal(err, undefined);

        for (var i in fields) {
          assert(typeof account[fields[i]] !== undefined);
        }

        done();
      });
    });

    it('should fail if containerNameHmacKey does not verify', function(done) {
      var account = setupAccount();

      // Modify the iv to provoke an invalid signature
      var iv = account.containerNameHmacKeyCiphertext.ciphertext.iv;
      account.containerNameHmacKeyCiphertext.ciphertext.iv = iv.substr(0, iv.length - 2) + 'AA';

      account.unravel(function(err) {
        assert(err !== undefined);
        done();
      });
    });

    it('should fail if hmacKey does not verify', function(done) {
      var account = setupAccount();

      // Modify the iv to provoke an invalid signature
      var iv = account.hmacKeyCiphertext.ciphertext.iv;
      account.hmacKeyCiphertext.ciphertext.iv = iv.substr(0, iv.length - 2) + 'AA';

      account.unravel(function(err) {
        assert(err !== undefined);
        done();
      });
    });

    it('should fail if secretKey does not verify', function(done) {
      var account = setupAccount();

      // Modify the mac slightly to provoke an invalid one
      var mac = account.keypairMac;
      account.keypairMac = mac.substr(0, mac.length - 2) + 'AA';

      account.unravel(function(err) {
        assert.notEqual(err, undefined);
        done();
      });
    });

    it('should fail if signKeyPrivate does not verify', function(done) {
      var account = setupAccount();

      // Modify the mac slightly to provoke an invalid one
      var mac = account.signKeyPrivateMac;
      account.signKeyPrivateMac = mac.substr(0, mac.length - 2) + 'AA';

      account.unravel(function(err) {
        assert.notEqual(err, undefined);
        done();
      });
    });

    it('should fail if pubKey is malformed', function(done) {
      var account = setupAccount();

      // we can't just munge the pubKey point here
      // because SJCL will just throw, so we have to
      // find another point on the curve
      var curve = sjcl.ecc.curves['c' + account.pubKey.curve];
      var newExponent = sjcl.bn.random(curve.r, crypton.paranoia);
      var newPoint = curve.G.mult(newExponent);
      account.pubKey.point = newPoint.toBits();

      account.unravel(function(err) {
        assert.equal(err, 'Server provided incorrect public key');
        done();
      });
    });

    it('should fail if signKeyPub is malformed', function(done) {
      var account = setupAccount();

      // we can't just munge the pubKey point here
      // because SJCL will just throw, so we have to
      // find another point on the curve
      var curve = sjcl.ecc.curves['c' + account.pubKey.curve];
      var newExponent = sjcl.bn.random(curve.r, crypton.paranoia);
      var newPoint = curve.G.mult(newExponent);
      account.signKeyPub.point = newPoint.toBits();

      account.unravel(function(err) {
        // XXX ecto
        // if the the public signing key is bad,
        // verifyAndDecrypt will fail to verify
        // containerNameHmacKey and hmacKey
        // inside work.unravelAccount
        assert.equal(err, 'Could not parse containerNameHmacKey');
        done();
      });
    });
  });

  describe('serialize()', function() {
    it('should return the correct fields', function() {
      var expected = [
        'srpVerifier',
        'srpSalt',
        'containerNameHmacKeyCiphertext',
        'hmacKeyCiphertext',
        'keypairCiphertext',
        'keypairMac',
        'pubKey',
        'keypairSalt',
        'keypairMacSalt',
        'signKeyPrivateMacSalt',
        'username',
        'signKeyPub',
        'signKeyPrivateCiphertext',
        'signKeyPrivateMac',
      ];

      var account = setupAccount();
      var serialized = account.serialize();
      assert.deepEqual(Object.keys(serialized), expected);
    });

    it('should return the correct values', function() {
      var account = setupAccount();
      var ret = account.serialize();
      assert.deepEqual(ret.containerNameHmacKeyCiphertext, account.containerNameHmacKeyCiphertext);
      assert.deepEqual(ret.srpVerifier, account.srpVerifier);
      assert.deepEqual(ret.srpSalt, account.srpSalt);
      assert.deepEqual(ret.hmacKeyCiphertext, account.hmacKeyCiphertext);
      assert.deepEqual(ret.keypairCiphertext, account.keypairCiphertext);
      assert.deepEqual(ret.keypairMac, account.keypairMac);
      assert.deepEqual(ret.pubKey, account.pubKey);
      assert.deepEqual(ret.challengeKeySalt, account.challengeKeySalt);
      assert.deepEqual(ret.keypairSalt, account.keypairSalt);
      assert.deepEqual(ret.keypairMacSalt, account.keypairMacSalt);
      assert.deepEqual(ret.signKeyPrivateMacSalt, account.signKeyPrivateMacSalt);
      assert.deepEqual(ret.username, account.username);
      assert.deepEqual(ret.signKeyPub, account.signKeyPub);
      assert.deepEqual(ret.signKeyPrivateCiphertext, account.signKeyPrivateCiphertext);
      assert.deepEqual(ret.signKeyPrivateMac, account.signKeyPrivateMac);
    });
  });
});

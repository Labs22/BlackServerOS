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

describe('Core', function() {
  before(function() {
    sjcl.random.setDefaultParanoia(0);
  });

  this.timeout(30000);

  describe('default properties', function() {
    it('should have the correct version', function() {
      var version = '0.1.0';
      assert.equal(crypton.version, version);
    });

    it('should have the correct host', function() {
      assert.equal(crypton.host, 'localhost');
    });

    it('should have the correct port', function() {
      assert.equal(crypton.port, '1025');
    });

    it('should have the correct default cipher mode', function() {
      assert.equal(crypton.cipherOptions.mode, 'gcm');
    });
  });

  describe('startCollectors()', function() {
    it('should trigger crypton collectorsStarted flag', function() {
      crypton.collectorsStarted = false;
      assert.doesNotThrow(crypton.startCollectors);
      assert.equal(crypton.collectorsStarted, true);
    });

    it('should trigger sjcl collectorsStarted flag', function() {
      crypton.collectorsStarted = false;
      assert.doesNotThrow(crypton.startCollectors);
      assert.equal(sjcl.random._collectorsStarted, true);
    });

    it('should add the expected listeners to the document object', function() {
      // I couldn't figure out how to actually test that
      // listeners were added. I tried to check document.onmousemove
      // etc, but they were always null. Could have something to do with
      // phantom? The listeners that it should add are here
      // https://github.com/bitwiseshiftleft/sjcl/blob/master/core/random.js#L243..L254
    });
  });

  describe('url()', function() {
    it('should return the correct url', function() {
      assert.equal(crypton.url(), 'https://localhost:1025');
    });
  });

  describe('randomBytes()', function() {
    it('should throw when given no input', function() {
      assert.throw(crypton.randomBytes, 'randomBytes requires input');
    });

    // the output of this function is an array of SJCL "words"
    // being 4 bytes each. the minimum outpt length is 1 word.
    it('should throw when given input < 4', function() {
      var err = null;

      try {
        crypton.randomBytes(3);
      } catch (e) {
        err = e;
      }

      assert.equal(err.message, 'randomBytes cannot return less than 4 bytes');
    });

    it('should throw when given non-numeric input', function() {
      var err = null;

      try {
        crypton.randomBytes('bitcoins');
      } catch (e) {
        err = e;
      }

      assert.equal(err.message, 'randomBytes requires integer input');
    });

    it('should throw when given numeric but non-integer input', function() {
      var err = null;

      try {
        crypton.randomBytes(4.5);
      } catch (e) {
        err = e;
      }

      assert.equal(err.message, 'randomBytes requires integer input');
    });

    it('should throw when given integer input of multiple other than 4', function() {
      var err = null;

      try {
        crypton.randomBytes(15);
      } catch (e) {
        err = e;
      }

      assert.equal(err.message, 'randomBytes requires input as multiple of 4');
    });

    it('should return an array', function() {
      var random = crypton.randomBytes(4);
      assert(Array.isArray(random));
    });

    it('should return the appropriate number of words for small nbytes', function() {
      var random = crypton.randomBytes(4);
      assert.equal(random.length, 1);
    });

    it('should return the appropriate number of words for larger nbytes', function() {
      var random = crypton.randomBytes(32);
      assert.equal(random.length, 8); // 32 / 4
    });

    it('should return different values on distinct calls', function() {
      var random = crypton.randomBytes(4);
      var random2 = crypton.randomBytes(4);
      assert.notEqual(random[0], random2[0]);
    });
  });

  describe('constEqual()', function() {
    it('should return true to same string', function() {
      assert(crypton.constEqual('somestring', 'somestring'));
    });

    it('should return false to same prefix but different length strings', function() {
      assert(!crypton.constEqual('somestring', 'somestringdifferent'));
    });

    it('should return false to same postfix but different length strings', function() {
      assert(!crypton.constEqual('differentsomestring', 'somestring'));
    });

    it('should return false for undefined', function() {
      assert(!crypton.constEqual('somestring', undefined));
    });

    it('should return false for int', function() {
      assert(!crypton.constEqual('somestring', 42));
    });

    it('should return false for array', function() {
      var testArray = [4, 2];
      assert(!crypton.constEqual('somestring', testArray));
    });
  });

  describe('randomBits()', function() {
    it('should throw when given no input', function() {
      assert.throw(crypton.randomBits, 'randomBits requires input');
    });

    it('should throw when given input < 32', function() {
      var err = null;

      try {
        crypton.randomBits(31);
      } catch (e) {
        err = e;
      }

      assert.equal(err.message, 'randomBits cannot return less than 32 bits');
    });

    it('should throw when given non-numeric input', function() {
      var err = null;

      try {
        crypton.randomBits('litecoin');
      } catch (e) {
        err = e;
      }

      assert.equal(err.message, 'randomBits requires integer input');
    });

    it('should throw when given numeric but non-integer input', function() {
      var err = null;

      try {
        crypton.randomBits(36.5);
      } catch (e) {
        err = e;
      }

      assert.equal(err.message, 'randomBits requires integer input');
    });

    it('should throw when given integer input of multiple other than 32', function() {
      var err = null;

      try {
        crypton.randomBits(43);
      } catch (e) {
        err = e;
      }

      assert.equal(err.message, 'randomBits requires input as multiple of 32');
    });

    it('should return the appropriate number of words for small nbits', function() {
      var random = crypton.randomBits(32);
      assert.equal(random.length, 1); // 4 bytes * 8 bits * 1 word = 32 bits
    });

    it('should return the appropriate number of words for larger nbits', function() {
      var random = crypton.randomBits(256);
      assert.equal(random.length, 8); // 4 bytes * 8 bits * 8 words = 256 bits
    });

    // any further tests would just be repeats of randomBytes tests
  });

  describe('generateAccount()', function() {
    var err;
    var user;

    before(function(done) {
      crypton.generateAccount('user', 'pass', function() {
        err = arguments[0];
        user = arguments[1];
        done();
      }, {
        save: false,
      });
    });

    it('should exist', function() {
      assert(typeof crypton.generateAccount === 'function');
    });

    it('should generate the correct data', function(done) {
      assert(err === null);
      assert(user !== undefined);

      var fields = [
        'username',
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
        'signKeyPub',
        'signKeyPrivateCiphertext',
        'signKeyPrivateMac',
      ];

      for (var i in fields) {
        assert(typeof user[fields[i]] === 'string');
      }

      done();
    });

    it('should generate the correct salt format', function(done) {
      assert.equal(err, null);
      assert.notEqual(user, undefined);

      var salt = user.srpSalt;
      var offset = -1;

      assert.equal(salt.charAt(0), '$');
      assert.equal(salt.charAt(1), '2');

      if (salt.charAt(2) === '$') {
        offset = 3;
      } else {
        var minor = salt.charAt(2);
        assert.equal(minor, 'a');
        assert.equal(salt.charAt(3), '$');
        offset = 4;
      }

      assert(salt.charAt(offset + 2) <= '$');

      done();
    });
  });

  describe('account authorization', function() {
    it('should exist', function() {
      assert(typeof crypton.authorize === 'function');
    });

    // TODO should we just test this functionality in the integration tests?:q
  });

  describe('hmac()', function() {
    it('should return the expected mac', function() {
      var expected = '23eddb487287e7a20b4dca249cb6bd0190bef7115cb0e31669748277e262baff';

      assert.equal(crypton.hmac('somekey', 'somedata'), expected);
    });
  });

  describe('hmacAndCompare()', function() {
    it('should be equal with the expected input', function() {
      var expected = '23eddb487287e7a20b4dca249cb6bd0190bef7115cb0e31669748277e262baff';

      assert(crypton.hmacAndCompare('somekey', 'somedata', expected));
    });

    it('should not be equal with different input data', function() {
      var expected = '23eddb487287e7a20b4dca249cb6bd0190bef7115cb0e31669748277e262baff';

      assert(!crypton.hmacAndCompare('somekey', 'somedata2', expected));
    });

    it('should not be equal with different key', function() {
      var expected = '23eddb487287e7a20b4dca249cb6bd0190bef7115cb0e31669748277e262baff';

      assert(!crypton.hmacAndCompare('somekey2', 'somedata', expected));
    });
  });
});

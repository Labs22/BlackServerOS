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

describe('Container', function() {
  describe('add()', function() {
    var container = new crypton.Container();

    it('should add a key to the keys array', function(done) {
      container.add('legit', function(err) {
        assert.equal(err, null);
        assert.deepEqual(container.keys.legit, {});
        done();
      });
    });

    it('should refuse to add an existing key', function(done) {
      container.add('legit', function(err) {
        assert.equal(err, 'Key already exists');
        done();
      });
    });
  });

  describe('get()', function() {
    var container = new crypton.Container();

    before(function(done) {
      container.add('legit', function() {
        done();
      });
    });

    it('should return the correct key', function(done) {
      container.get('legit', function(err, legit) {
        assert.equal(err, null);
        assert.deepEqual(container.keys.legit, legit);
        done();
      });
    });

    it('should refuse to return a nonexistent key', function(done) {
      container.get('grail', function(err) {
        assert.equal(err, 'Key does not exist');
        done();
      });
    });
  });

  describe('save()', function() {
    it('should err if the container hasn\'t changed', function(done) {
      var container = new crypton.Container();
      container.save(function(err) {
        assert.equal(err, 'Container has not changed');
        done();
      });
    });

    it('should add a new version', function(done) {
      var container = new crypton.Container();
      var now = container.version;

      // is there a better place to do this?
      // perhaps in a before()
      var SIGN_KEY_BIT_LENGTH = 384;
      var signingKeys = sjcl.ecc.ecdsa.generateKeys(SIGN_KEY_BIT_LENGTH, crypton.paranoia);

      // these are necessary for operations, however
      // typically you will allow the session to create
      // a container instead of doing it manually
      container.name = 'legit';

      // sessionKey content doesn't matter in this test because
      // we're not testing the contents of the ciphertext
      container.sessionKey = [1, 2, 3, 4, 5, 6, 7, 8];
      container.session = {};
      container.session.account = {};

      // containerNameHmacKey content doesn't matter here either
      // because we aren't testing getPublicName() functionality
      container.session.account.containerNameHmacKey = [1, 2, 3, 4, 5, 6, 7, 8];
      container.session.account.signKeyPrivate = signingKeys.sec;

      container.add('foo', function() {
        container.get('foo', function(err, foo) {
          foo.bar = 'baz';

          container.save(function(err) {
            assert.notEqual(container.version > now);

            var newVersion = container.versions[container.version];
            assert.deepEqual(newVersion.foo, {
              bar: 'baz',
            });

            done();
          }, {
            save: false,
          });
        });
      });
    });

    it('should generate the correct transaction chunk', function(done) {
      var container = new crypton.Container();

      // is there a better place to do this?
      // perhaps in a before()
      var SIGN_KEY_BIT_LENGTH = 384;
      var signingKeys = sjcl.ecc.ecdsa.generateKeys(SIGN_KEY_BIT_LENGTH, crypton.paranoia);

      // these are necessary operations, however
      // typically you will allow the session to create
      // a container instead of doing it manually
      container.name = 'legit';

      // sessionKey content doesn't matter in this test because
      // we're not testing the contents of the ciphertext, only
      // that it was successfully created
      container.sessionKey = [1, 2, 3, 4, 5, 6, 7, 8];
      container.session = {};
      container.session.account = {};

      // containerNameHmacKey content doesn't matter here either
      // because we aren't testing getPublicName() functionality
      container.session.account.containerNameHmacKey = [1, 2, 3, 4, 5, 6, 7, 8];
      container.session.account.signKeyPrivate = signingKeys.sec;

      container.add('foo', function() {
        container.get('foo', function(err, foo) {
          foo.bar = 'baz';

          container.save(function(err, chunk) {
            assert.equal(err, null);
            assert.equal(chunk.type, 'addContainerRecord');

            assert.deepEqual(Object.keys(JSON.parse(chunk.payloadCiphertext.ciphertext)), [
              'iv',
              'v',
              'iter',
              'ks',
              'ts',
              'mode',
              'adata',
              'cipher',
              'ct',
            ]);

            // let's test that the signature exists and looks like a signature
            // we can't test the content because the keys will change
            var expectedSignatureLength = 24;
            assert.equal(chunk.payloadCiphertext.signature instanceof Array, true);
            assert.equal(chunk.payloadCiphertext.signature.length, expectedSignatureLength);

            done();
          }, {
            save: false,
          });
        });
      });
    });
  });

  describe('getDiff()', function() {
    // TODO this would be a good place to
    // test more complex interactions!
    it('should get the correct diff', function(done) {
      var container = new crypton.Container();

      container.add('foo', function() {
        container.get('foo', function(err, foo) {
          foo.bar = 'baz';

          container.getDiff(function(err, diff) {
            assert.equal(err, null);
            assert.deepEqual(diff, {
              foo: [
                { bar: 'baz' },
              ],
            });
            done();
          });
        });
      });
    });

    it('should callback with null on an unchanged container', function(done) {
      var container = new crypton.Container();
      container.getDiff(function(err, diff) {
        assert.equal(err, null);
        assert.equal(diff, null);
        done();
      });
    });
  });

  describe('getVersions()', function() {
    it('should return an array', function() {
      var container = new crypton.Container();
      var ret = container.getVersions();
      assert.isArray(ret);
    });

    it('should return the correct array', function() {
      var container = new crypton.Container();
      var now = +new Date();
      container.versions[now] = {};
      var ret = container.getVersions();
      assert.deepEqual(ret, [now + '']);

      // version keys will always be strings
    });
  });

  describe('getVersion()', function() {
    it('should return the correct object', function() {
      var container = new crypton.Container();
      var now = +new Date();
      var expected = {
        foo: 'bar',
      };
      container.versions[now] = expected;
      var ret = container.getVersion(now);
      assert.deepEqual(ret, expected);
    });
  });

  describe('latestVersion()', function() {
    it('should return the latest version timestamp', function() {
      var container = new crypton.Container();
      var last;

      for (var i = 0; i < 1000; i++) {
        last = +new Date();
        container.versions[last] = {};
      }

      assert.equal(container.latestVersion(), last);
    });
  });

  describe('getPublicName()', function() {
    it('should return the correct hmac for a standard name', function() {
      var container = new crypton.Container();

      // these are necessary for the encryption
      // typically you will allow the session to create
      // a container instead of doing it manually
      container.name = 'legit';
      container.session = {};
      container.session.account = {};
      container.session.account.containerNameHmacKey = [1193696192, 274367050, -1647541843, -73767300, 1167252974, -984408945, -1161509559, 962393744];

      var expected = 'a6b4fdf950fb02d66bee39fbe8af8456a615969275a2ec0db0fc09936b99e882';
      assert.equal(container.getPublicName(), expected);
    });
  });

  describe('parseHistory()', function() {
    // TODO this should go in the integration tests
    // because it requires a valid session
  });

  describe('decryptRecord()', function() {
    // TODO this should go in the integration tests
    // because it requires a valid session
  });

  describe('getHistory()', function() {
    // TODO this should go in the integration tests
    // because it requires a valid session
  });

  describe('sync()', function() {
    // TODO this should go in the integration tests
    // because it requires a valid session
  });
});

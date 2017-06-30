/* Crypton Server, Copyright 2013 SpiderOak, Inc.
 *
 * This file is part of Crypton Server.
 *
 * Crypton Server is free software: you can redistribute it and/or modify it
 * under the terms of the Affero GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Crypton Server is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Affero GNU General Public
 * License for more details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * along with Crypton Server.  If not, see <http://www.gnu.org/licenses/>.
*/

'use strict';

var app = require('../app');
var assert = require('assert');
var should = require('chai').should();
var Account = require('../lib/account');

describe('Account model', function () {
  describe('save()', function () {
    it('should save valid accounts', function (done) {
      var account = new Account();

      var requestedAccount = {
        username: 'pizza',
        keypairSalt: '[1,2,3]',
        keypairMacSalt: '[1,2,3]',
        keypairCiphertext: { keypair: 'ciphertext' },
        keypairMac: '[1,2,3]',
        pubKey: { pub: 'key' },
        srpSalt: 'saltstring',
        srpVerifier: 'verifierstring',
        containerNameHmacKeyCiphertext: '[1,2,3]',
        hmacKeyCiphertext: '[1,2,3]',
        signKeyPub: { pub: 'key' },
        signKeyPrivateMacSalt: '[1,2,3]',
        signKeyPrivateCiphertext: '[1,2,3]',
        signKeyPrivateMac: '[1,2,3]'
      };

      account.update(requestedAccount);

      account.save(function (err) {
        if (err) { throw err; }
        done();
      });
    });

    it('should not save accounts with invalid usernames', function (done) {
      var account = new Account();

      var requestedAccount = {
        username: 'pizza /@',
        keypairSalt: '[1,2,3]',
        keypairCiphertext: { keypair: 'ciphertext' },
        pubKey: { pub: 'key' },
        srpSalt: 'saltstring',
        srpVerifier: 'verifierstring',
        containerNameHmacKeyCiphertext: '[1,2,3]',
        hmacKeyCiphertext: '[1,2,3]',
        signKeyPub: { pub: 'key' },
        signKeyPrivateCiphertext: '[1,2,3]'
      };

      account.update(requestedAccount);

      account.save(function (err) {
        assert(err === 'Username is not valid: it is not alphanumeric!');
        done();
      });
    });

    it('should not save accounts with usernames longer than 32 char', function (done) {
      var account = new Account();

      var requestedAccount = {
        username: 'pizzapizzapizzapizzapizzapizzapizzapizza',
        keypairSalt: '[1,2,3]',
        keypairCiphertext: { keypair: 'ciphertext' },
        pubKey: { pub: 'key' },
        srpSalt: 'saltstring',
        srpVerifier: 'verifierstring',
        containerNameHmacKeyCiphertext: '[1,2,3]',
        hmacKeyCiphertext: '[1,2,3]',
        signKeyPub: { pub: 'key' },
        signKeyPrivateCiphertext: '[1,2,3]'
      };

      account.update(requestedAccount);

      account.save(function (err) {
        assert(err === 'Username is not valid: exceeds 32 charcters!');
        done();
      });
    });

    it('should err out for empty accounts', function (done) {
      var account = new Account();

      account.save(function (err) {
        assert(err === 'undefined is not a valid username');
        done();
      });
    });
  });

  describe('get()', function () {
    it('should retrieve account object from database', function (done) {
      var account = new Account();
      var expectedProperties = [
        'username',
        'accountId',
        'keyringId',
        'keypairSalt',
        'keypairMacSalt',
        'keypairCiphertext',
        'keypairMac',
        'pubKey',
        'srpVerifier',
        'srpSalt',
        'containerNameHmacKeyCiphertext',
        'hmacKeyCiphertext',
        'signKeyPub',
        'signKeyPrivateMacSalt',
        'signKeyPrivateCiphertext',
        'signKeyPrivateMac'
      ];

      account.get('pizza', function (err) {
        if (err) { throw err; }
        assert.deepEqual(expectedProperties, Object.keys(account.toJSON()));
        done();
      });
    });

    it('should callback with error if given nonexistant username', function (done) {
      var account = new Account();
      account.get('pizzasaurus', function (err) {
        assert.equal(err, 'Account not found.');
        done();
      });
    });
  });

  describe('update()', function () {
    it('should update the account by key/value', function () {
      var account = new Account();
      account.update('foo', 'bar');
      assert.equal(account.foo, 'bar');
    });

    it('should update the account with an object of key/value pairs', function () {
      var account = new Account();
      account.update({
        foo: 'bar',
        bar: 'baz'
      });
      assert.equal(account.foo, 'bar');
      assert.equal(account.bar, 'baz');
    });
  });

  describe('beginSrp()', function () {
    var srpA = '9d57597a70ef0ee63d066cf49e4620332c05704b8b065b906108d8835d9b88e14e6b4a07bdda649ef1f8a09ad19d56d71236ee71abd14fc9fa5f3e73136aeed7f5029bff0c8f27b584bc90afb6d1329d3bb64ad2d5e754e6a10ab30d7a8e8d0b9c959ebd145005d8cc70d0a5ffa94db270d575239a0fb1f6310ea3d1cd62f2dc75aa597518ff84c9fd9fb41e7c44b9af405d8066beebdebb29f95b25d3b90a685cbec30340d601afc07eda7edd125b522adcce2128b9c35749a751cdfe3ccb3ed3b6bebacff86e18c0af9d26034bbd23921174c4834690b52b7cb34bc45d5793d46ea0663acbc817681128b90225ab9a922190dbeb8a6079d6ddbe05f90fc9d3';

    it('should generate an SRP B value from a supplied SRP A', function (done) {
      var account = new Account();
      account.update({srpVerifier: '00'});

      account.beginSrp(srpA, function (err, srpParams) {
        if (err) { throw err; }
        assert(srpParams);
        assert.equal(typeof srpParams.B, 'string');
        assert.equal(srpParams.B.length, 512); // 256-bit pubkey length
        done();
      });
    });

    it('should fail if there is no SRP A input', function (done) {
      var account = new Account();

      account.beginSrp(null, function (err) {
        assert.equal(err, 'Invalid SRP A value.');
        done();
      });
    });

    it('should fail if the SRP A input is the wrong size', function(done) {
      var account = new Account();

      account.beginSrp('blargh', function(err) {
        assert.equal(err, 'Invalid SRP A value.');
        done();
      });
    });

    it('should fail with badly formed srpA values', function (done) {
      var account = new Account();
      var srpArr = [];
      for (var i = 0; i < 512; i++) {
        srpArr.push(0);
      }

      var srpA = srpArr.join('');
      var verifier = '21b37591ff48766be9b93d18d34ccace802c37f8889c61f2bc374809458f0c4c42baf45cae4d48366006abb25ddc3ac5aa651a9779e3e9fe3916f2be28f86884610d75ade8b08508483adb9fef1ba28eec56322f356cea145047c7d3c1309dac1253aabe08446f23b63c7062ae9cf0bee503bcd781536f4b21d27dc96d787e2a31dc74c30a7544e779429f3ee9fd508e52c8ddf02d311b79682870f44053b79b62198f780178f571f42e9a546cb8507a92aa2542103474ae63564f1194c144f444a008936a031c959e1d4b5dddea40ead43a75ee654b5179ce29cb7d5e6a1124e582c6240822eca10fb727eee198456f4c3f95b6b579f57dff0904347e6467c9';
      account.update({ srpVerifier: verifier });
      // XXXddahl: We are really testing contnueSrp where the client sends
      // invalid srpA values, which make it past our initial validation
      // of if(string and 512 chars)
      account.beginSrp(srpA, function (err, srpB) {
        assert.equal(err, 'srpA is invalid');
        done();
      });
    });
  });

  describe('checkSrp()', function () {
    // verifier for 'alice' / 'password123'
    // The associated salt is 000102030405060708090a0b0c0d0e0f
    var verifier = '21b37591ff48766be9b93d18d34ccace802c37f8889c61f2bc374809458f0c4c42baf45cae4d48366006abb25ddc3ac5aa651a9779e3e9fe3916f2be28f86884610d75ade8b08508483adb9fef1ba28eec56322f356cea145047c7d3c1309dac1253aabe08446f23b63c7062ae9cf0bee503bcd781536f4b21d27dc96d787e2a31dc74c30a7544e779429f3ee9fd508e52c8ddf02d311b79682870f44053b79b62198f780178f571f42e9a546cb8507a92aa2542103474ae63564f1194c144f444a008936a031c959e1d4b5dddea40ead43a75ee654b5179ce29cb7d5e6a1124e582c6240822eca10fb727eee198456f4c3f95b6b579f57dff0904347e6467c9';
    var srpb = new Buffer('65f472bd0b070a766ced6c18657fd5d9ac246a80ec19e46f9794a4864a1f0f4f', 'hex');

    it('should callback without error on correct input', function (done) {
      var account = new Account();
      account.update({srpVerifier: verifier});

      // Client's generated A value
      // The corresponding a value is
      // 10b73a33e22c819169a3a316ff15bfb043eff0e6b974618c2ec968f824f425f0
      var srpA = '3c8f5244f6942995ced27dbe38d68bf3d10f2eb051d1c8e62f1d234d995cdbead2e330fbc1ceeb59953d357a23de80fb9ce9213dec251bf76b1234825ff593489257873aa9ad76d995015e71e9cbbcf846a9b50fbc125cc9d55f4bdfbe2dfc1fecb1710c70942f9b17327b49d73009c977a599b04fd5607b0f1b63810f02b6690e900a3d6d9e86b2a33dbd71e4f9a4b97a7fe8af305f471502401da7edbf567462b1c0a40e98fcd0c5b80919f7d1a382839de2ef3a2236cab81c00df07d6905316f8409f533f9aff48dde84bfcf86f8c6751f30b3e202b67adac71566f605881caf6e3466d4de8553fa0506c25362bf88cca61a21c440f2b1161a8b36389b217';

      // We skip the initial b generation in beginSrp() and instead supply our
      // own to continueSrp()
      account.continueSrp(srpA, srpb, function (err, srpParams) {
        if (err) { throw err; }

        assert.equal(srpParams.B, '7bc17cdb11c1b76af3969c210bf3ecaecf27164da0d8ad4f775e3757e2cc743fada866e969e2a334356db0f2f1993ce0d71a03fc563e4db9784283fe5336f1a17dfe5dc174f0c16d3f3cc951d87026e7c3554fb361ceb7cf4ec57252706efce44b8444ecb43e82c36172e8e29f35aedc32dd9b7b02c71003eeb85cc3961d920b2a0c1478b4bf1d1bd77f7d5130345123485da76c8de57fe8d18f6c6f08c578370bb972c76c6caf98f586072bcafd9794eb890273f29c0b7e1cc16a19b81760b64b4810039aed814865fc8fcbf5c83493fa2fafc696861fce9d1c85345f9ce0fd56c7814d32a6dabfe7fdb714c6e42554ae975d621aeded45f9fe12784bad5ac3');
        // We've now generated our B value and sent it back to the client. They
        // give us back an M1 verification message.
        var srpM1 = '14824a6b7e6d68399db75e9d4c48079587607a425033c574592c002a9f3ba7ff';
        account.checkSrp(srpParams, srpM1, function (err, srpM2) {
          if (err) { throw err; }
          assert.equal(srpM2.toString('hex'),
                       '646921a2c9b45a53b913eadae3a79212158607079bac7c1caff9691805866057');
          done();
        });
      });
    });

    it('should callback with err on wrong password', function (done) {
      var account = new Account();
      account.update({srpVerifier: verifier});

      // SRP A value using alice / badpassword
      // SRP a value is the same as the valid exchange above
      var srpA = '3c8f5244f6942995ced27dbe38d68bf3d10f2eb051d1c8e62f1d234d995cdbead2e330fbc1ceeb59953d357a23de80fb9ce9213dec251bf76b1234825ff593489257873aa9ad76d995015e71e9cbbcf846a9b50fbc125cc9d55f4bdfbe2dfc1fecb1710c70942f9b17327b49d73009c977a599b04fd5607b0f1b63810f02b6690e900a3d6d9e86b2a33dbd71e4f9a4b97a7fe8af305f471502401da7edbf567462b1c0a40e98fcd0c5b80919f7d1a382839de2ef3a2236cab81c00df07d6905316f8409f533f9aff48dde84bfcf86f8c6751f30b3e202b67adac71566f605881caf6e3466d4de8553fa0506c25362bf88cca61a21c440f2b1161a8b36389b217';

      account.beginSrp(srpA, function (err, srpParams) {
        var srpM1 = 'f6861abf8e5fa421266b04b281a4093de2b4951b86202ab37b7348edc1c30ee0';
        account.checkSrp(srpParams, srpM1, function (err, srpM2) {
          assert.equal(err, 'Incorrect password');
          assert.equal(srpM2, undefined);
          done();
        });
      });
    });
  });

  describe('toJSON()', function () {
    it('should return an object', function () {
      var account = new Account();
      var ret = account.toJSON();
      assert.equal(typeof ret, 'object');
    });

    it('should return account properties', function () {
      var account = new Account();
      account.update('foo', 'bar');
      var ret = account.toJSON();
      assert.equal(ret.foo, 'bar');
    });
  });
});

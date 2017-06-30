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

var should = require('chai').should();
var assert = require('assert');
var Transaction = require('../lib/transaction');

describe('Transaction model', function () {
  var transactionId;

  describe('#create()', function () {
    it('should add a transactionId to the object', function (done) {
      var accountId = 1; // account from account lib tests
      var tx = new Transaction();
      tx.create(accountId, function (err) {
        assert.equal(err, null);
        tx.transactionId.should.exist;
        transactionId = tx.transactionId;
        done();
      });
    });
  });

  describe('#get()', function () {
    it('should err on invalid transactionId', function (done) {
      var tx = new Transaction();
      tx.get(666, function (err) {
        assert.equal(err, 'Transaction does not exist');
        done();
      });
    });

    it('should get the created transaction', function (done) {
      var tx = new Transaction();
      tx.get(transactionId, function (err) {
        assert.equal(err, null);
        assert.equal(tx.numOperations, 0);
        done();
      });
    });
  });

  describe('#update()', function () {
    it('should update the transaction by key/value', function () {
      var tx = new Transaction();
      tx.update('foo', 'bar');
      assert.equal(tx.foo, 'bar');
    });

    it('should update the transaction with an object of key/value pairs', function () {
      var tx = new Transaction();
      tx.update({
        foo: 'bar',
        bar: 'baz'
      });
      assert.equal(tx.foo, 'bar');
      assert.equal(tx.bar, 'baz');
    });
  });

  describe('#add()', function () {
    // it('should refuse a non-standard operation', function (done) {
    //   var tx = new Transaction();

    //   tx.update('interactingAccount', 1);

    //   tx.get(transactionId, function (err) {
    //     assert.equal(err, null);

    //     var chunk = {};

    //     tx.add(chunk, function (err) {
    //       assert.equal(err, 'Invalid transaction type');
    //       done();
    //     });
    //   });
    // });
    // XXXddahl: commented out in preparation for excising transactions and containers
    it('should refuse to add transaction chunks if the transaction doesn\'t belong to the account', function (done) {
      var tx = new Transaction();

      tx.update('interactingAccount', 666);

      tx.get(transactionId, function (err) {
        assert.equal(err, null);

        var chunk = {
          type: 'addContainer',
          containerNameHmac: 'exists'
        };

        tx.add(chunk, function (err) {
          assert.equal(err, 'Transaction does not belong to account');
          done();
        });
      });
    });

    // it('should execute valid addContainer request', function (done) {
    //   var tx = new Transaction();

    //   tx.update('interactingAccount', 1);

    //   tx.get(transactionId, function (err) {
    //     assert.equal(err, null);

    //     var chunk = {
    //       type: 'addContainer',
    //       containerNameHmac: 'exists'
    //     };

    //     tx.add(chunk, function (err) {
    //       assert.equal(err, null);
    //       done();
    //     });
    //   });
    // }); // Excising transactions & containers

    // it('should execute valid addContainerSessionKey request', function (done) {
    //   var tx = new Transaction();

    //   tx.update('interactingAccount', 1);

    //   tx.get(transactionId, function (err) {
    //     assert.equal(err, null);

    //     var chunk = {
    //       type: 'addContainerSessionKey',
    //       containerNameHmac: 'exists',
    //       signature: 'herp'
    //     };

    //     tx.add(chunk, function (err) {
    //       assert.equal(err, null);
    //       done();
    //     });
    //   });
    // }); // Excising transactions & containers

    // it('should execute valid addContainerSessionKeyShare request', function (done) {
    //   var tx = new Transaction();

    //   tx.update('interactingAccount', 1);

    //   tx.get(transactionId, function (err) {
    //     assert.equal(err, null);

    //     var chunk = {
    //       type: 'addContainerSessionKeyShare',
    //       containerNameHmac: 'exists',
    //       sessionKeyCiphertext: { sessionKey: 'ciphertext' },
    //       hmacKeyCiphertext: { hmacKey: 'ciphertext' }
    //     };

    //     tx.add(chunk, function (err) {
    //       assert.equal(err, null);
    //       done();
    //     });
    //   });
    // }); // Excising transactions & containers

    // it('should execute addContainerRecord', function (done) {
    //   var tx = new Transaction();

    //   tx.update('interactingAccount', 1);

    //   tx.get(transactionId, function (err) {
    //     assert.equal(err, null);

    //     var chunk = {
    //       type: 'addContainerRecord',
    //       containerNameHmac: 'exists',
    //       latestRecordId: 123,
    //       payloadCiphertext: { payload: 'ciphertext' }
    //     };

    //     tx.add(chunk, function (err) {
    //       assert.equal(err, null);
    //       done();
    //     });
    //   });
    // }); // Excising transactions & containers
  });

  describe('#commit()', function () {
    it('should refuse to commit transactions not belonging to current account', function (done) {
      var tx = new Transaction();
      tx.get(transactionId, function (err) {
        assert.equal(err, null);

        tx.abort(function (err) {
          assert.equal(err, 'Transaction does not belong to account');
          done();
        });
      });
    });

    // it('should commit known transaction', function (done) {
    //   var tx = new Transaction();

    //   tx.update('interactingAccount', 1);

    //   tx.get(transactionId, function (err) {
    //     assert.equal(err, null);

    //     tx.requestCommit(function (err) {
    //       assert.equal(err, null);

    //       setTimeout(function () {
    //         process.finishedTransaction = true;
    //         done();
    //       }, 1000);
    //     });
    //   });
    // }); // Excising transactions & containers
  });

  describe('#abort()', function () {
    it('should refuse to abort transactions not belonging to current account', function (done) {
      var tx = new Transaction();
      tx.get(transactionId, function (err) {
        assert.equal(err, null);

        tx.abort(function (err) {
          assert.equal(err, 'Transaction does not belong to account');
          done();
        });
      });
    });

    // it('should abort a transaction if it belongs to account', function (done) {
    //   var tx = new Transaction();

    //   tx.update('interactingAccount', 1);

    //   tx.get(transactionId, function (err) {
    //     assert.equal(err, null);
    //     assert.equal(tx.abortTimestamp, null);

    //     tx.abort(function (err) {
    //       assert.equal(err, null);

    //       tx.get(transactionId, function (err) {
    //         assert(tx.abortTimestamp != null);
    //         done();
    //       });
    //     });
    //   });
    // });
  });
});

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

describe('Transaction functionality', function () {
  this.timeout(15000);

  var session;
  var tx;

  before(function (done) {
    crypton.authorize('notSoSmart', 'pass', function (err, rawSession) {
      session = rawSession;
      done();
    });
  });

  describe('create()', function () {
    it('should create a transaction', function (done) {
      tx = new crypton.Transaction(session, function (err) {
        assert.equal(err, null);
        assert.equal(tx.id, 14);
        done();
      });
    });
  });

  describe('saveChunk()', function () {
    it('should refuse valid transaction type but invalid data', function (done) {
      var chunk = {
        type: 'addContainer'
      };

      tx.saveChunk(chunk, function (err) {
        assert.equal(err, 'Invalid chunk data');
        done();
      });
    });

    it('should accept chunk withvalid data', function (done) {
      var chunk = {
        type: 'addContainer',
        containerNameHmac: 'foo'
      };

      tx.saveChunk(chunk, function (err) {
        assert.equal(err, null);
        done();
      });
    });
  });

  describe('save()', function () {
    it('should accept a single valid chunk argument with a callback', function (done) {
      var chunk = {
        type: 'addContainer',
        containerNameHmac: 'bar'
      };

      tx.save(chunk, function (err) {
        assert.equal(err, null);
        done();
      });
    });

    it('should accept multiple valid chunk arguments with a callback', function (done) {
      var chunk = {
        type: 'addContainer',
        containerNameHmac: 'baz'
      };

      var chunk2 = {
        type: 'addContainer',
        containerNameHmac: 'booey'
      };

      tx.save(chunk, chunk2, function (err) {
        assert.equal(err, null);
        done();
      });
    });

    it('should accept a single valid chunk argument without callback', function (done) {
      var chunk = {
        type: 'addContainer',
        containerNameHmac: 'barfoo'
      };

      var err = null;
      try {
        tx.save(chunk);
      } catch (e) {
        err = e;
      }

      assert.equal(err, null);
      done();
    });

    it('should accept multiple valid chunk arguments without a callback', function (done) {
      var chunk = {
        type: 'addContainer',
        containerNameHmac: 'bazbar'
      };

      var chunk2 = {
        type: 'addContainer',
        containerNameHmac: 'booeyfoo'
      };

      var err = null;
      try {
        tx.save(chunk, chunk2);
      } catch (e) {
        err = e;
      }

      assert.equal(err, null);
      done();
    });

    it('should refuse a single invalid chunk argument with a callback', function (done) {
      var chunk = {
        type: 'addContainer'
      };

      tx.save(chunk, function (err) {
        assert.equal(err, 'Invalid chunk data');
        done();
      });
    });

    it('should refuse multiple invalid chunk arguments with a callback', function (done) {
      var chunk = {
        type: 'addContainer'
      };

      var chunk2 = {
        type: 'addContainer'
      };

      tx.save(chunk, chunk2, function (err) {
        assert.equal(err, 'Invalid chunk data');
        done();
      });
    });
  });

  describe('commit()', function () {
    it('should process transaction with errors', function (done) {
      tx.commit(function (err) {
        assert.equal(err, 'Malformed transaction');
        done();
      });
    });

    it('should refuse a commit request for invalid transaction', function (done) {
      var tx2 = new crypton.Transaction(session);
      tx2.id = 'flabbergast';
      tx2.commit(function (err) {
        assert.equal(err, 'Transaction does not belong to account');
        done();
      });
    });
  });

  describe('abort()', function () {
    it('should do something', function (done) {
      done();
    });
  });
});

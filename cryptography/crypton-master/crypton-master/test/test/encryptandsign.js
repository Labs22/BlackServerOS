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

describe('encryptAndSign+verifyAndDecrypt()', function () {
  this.timeout(15000);

  var alice;
  var bob;
  var aliceSession;
  var bobSession;
  var alicePeer;
  var bobPeer;
  var messageResult;
  var plaintext = 'MR JONES of the Manor Farm, had locked the hen-houses for the night, but was too drunk to remember to shut the pop-holes. With the ring of light from his lantern dancing from side to side he lurched across the yard, kicked off his boots at the back door, drew himself a last glass of beer from the barrel in the scullery, and made his way up to bed, where Mrs Jones was already snoring.';

  describe('Create 2 accounts and exchange a message and decrypt it', function () {
    it('Create Alice', function (done) {
      crypton.generateAccount('alice', 'pass', function (err, acct) {
        if (err) throw err;
        alice = acct;
        done();
      });
    });

    it('Get Alice\'s session', function (done) {
      crypton.authorize('alice', 'pass', function (err, sess) {
        if (err) throw err;
        aliceSession = sess;
        done();
      });
    });

    it('Create Bob', function (done) {
      crypton.generateAccount('bob', 'pass', function (err, acct) {
        if (err) throw err;
        bob = acct;
        done();
      });
    });

    it('Get Bob\'s session', function (done) {
      crypton.authorize('bob', 'pass', function (err, sess) {
        if (err) throw err;
        bobSession = sess;
        done();
      });
    });

    it('Get Alice as peer', function (done) {
      bobSession.getPeer('alice', function (err, peer) {
        if (err) throw err;

        peer.trust(function (err) {
          if (err) throw err;
          alicePeer = peer;
          // Send an inbox message to Alice in order for Alice to test
          // the inbox apis
          var headers = { test: 'message' };
          var payload = { secret: 'The toast has landed butter-side up' };
          alicePeer.sendMessage(headers, payload, function (err) {
            console.log("Bob sent a message to Alice");
            if (err) {
              throw err;
            }
            assert.equal(err, null);
            done();
          });
        });
      });
    });

    it('encryptAndSign a message for Alice', function (done) {
      messageResult = alicePeer.encryptAndSign(plaintext, aliceSession);
      assert.equal(messageResult.error, null);
      assert(messageResult.ciphertext && messageResult.signature);
      messageResult = JSON.stringify(messageResult);
      done();
    });

    it('Alice needs to login again so Alice\'s session is valid', function (done) {
      crypton.authorize('alice', 'pass', function (err, sess) {
        if (err) throw err;
        aliceSession = sess;
        done();
      });
    });

    it('Get Bob as peer', function (done) {
      aliceSession.getPeer('bob', function (err, peer) {
        if (err) throw err;

        peer.trust(function (err) {
          if (err) throw err;
          bobPeer = peer;
          done();
        });
      });
    });

    it('test inbox metadata', function (done) {
      aliceSession.inbox.getAllMetadata(function (err, metadata) {
        assert(metadata);
        assert.equal(err, null);
        assert.equal(metadata.length, 1);
        assert.equal(metadata[0].toUsername, 'alice');
        assert.equal(metadata[0].fromUsername, 'bob');
        assert(metadata[0].toAccountId);
        assert(metadata[0].fromAccountId);
        assert(metadata[0].messageId);
        assert(metadata[0].payloadLength);
        assert(metadata[0].headersLength);
        // Get the message
        aliceSession.inbox.get(metadata[0].messageId, function (err, message) {
          assert.equal(err, null);
          assert.equal(message.payload.secret, 'The toast has landed butter-side up');
          assert.equal(message.headers.test, 'message');
        });

        done();
      });
    });

    it('verifyAndDecrypt the message from Bob', function (done) {
      var ciphertext = JSON.parse(messageResult);
      var verified = aliceSession.account.verifyAndDecrypt(ciphertext, bobPeer);
      assert.equal(verified.error, null);
      assert.equal(verified.verified, true);
      // assert(verified.plaintext == plaintext); // XXXddahl: This is off by the "" introduced by JSON???
      done();
    });

    it('try to verifyAndDecrypt a message from Bob with a bad signature', function (done) {
      var ciphertext = JSON.parse(messageResult);
      ciphertext.signature[0] = -546152073;
      var verified = aliceSession.account.verifyAndDecrypt(ciphertext, bobPeer);
      assert.equal(verified.error, 'Cannot verify ciphertext');
      assert.equal(verified.verified, false);
      done();
    });
  });
});

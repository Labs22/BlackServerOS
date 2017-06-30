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

describe('test inbox polling for retrieving messages when the receiver was offline when the message has been sent', function () {
  this.timeout(150000);

  var alice;
  var bob;
  var aliceSession;
  var bobSession;
  var alicePeer;
  var bobPeer;
  var secretMsg = 'foo foo';

  describe('Create 2 accounts', function () {
    it('Create Alice', function (done) {
      crypton.generateAccount('alice', 'pass', function (err, acct) {
        if (err) throw err;
        alice = acct;
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
          // the inbox polling
          var headers = { test: 'message' };
          var payload = { secret: secretMsg };
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
  });

  describe('Poll the message from the inbox', function () {	
    var stringify = JSON.stringify;

    before(function (done) {
      var that = this;

      crypton.authorize('alice', 'pass', function (err, session) {
        if (err) throw err;
        aliceSession = session;
        done();
      });

      // this stringify function is to avoid the following error in mocha: 
      // Uncaught TypeError: JSON.stringify cannot serialize cyclic structures
      JSON.stringify = function(obj) {
        var seen = [];
        return stringify(obj, function(key, val) {
          if (typeof val === "object") {
            if (seen.indexOf(val) >= 0) { return; }
            seen.push(val);
          }
            return val;
        });
      };
    });

    it('trust bob', function (done) {
      aliceSession.getPeer('bob', function (err, peer) {
        if (err) throw err;

        peer.trust(function (err) {
          if (err) throw err;
          done();
        });
      });
    });

    it('poll', function (done) {
      aliceSession.inbox.poll(function (err, messages){

	Object.keys(messages).forEach(function (key) {
	  // only one message expected
	  var message = messages[key]
  	  assert.equal(message.payload['secret'], secretMsg);
 	})
	done();
      });
    });

    after(function() {
      JSON.stringify = stringify;
    });
  });
});

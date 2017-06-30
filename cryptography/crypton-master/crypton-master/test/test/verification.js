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

describe('Public key verification', function () {
  this.timeout(15000);
  var session;

  before(function (done) {
    crypton.authorize('notSoSmart', 'pass', function (err, rawSession) {
      if (err) throw err;
      session = rawSession;
      done();
    });
  });

  describe('Account object', function () {
    it('should have fingerprint property', function () {
      assert.isDefined(session.account.fingerprint);
    });
  });

  describe('Internal trusted peers item', function () {
    var peers;

    before(function (done) {
      session.getOrCreateItem(crypton.trustedPeers,
      function (err, trustedPeers) {
        if (err) throw err;
        peers = trustedPeers.value;
        done();
      });
    });

    it('should exist', function () {
      assert.isDefined(peers);
    });

    it('should be empty for a new account', function () {
      assert.deepEqual(peers, {});
    });
  });

  describe('Peer object', function () {
    var peer;

    before(function (done) {
      session.getPeer('notSoSmart', function (err, rawPeer) {
        if (err) throw err;
        peer = rawPeer;
        done();
      });
    });

    it('should have fingerprint property', function () {
      assert.isDefined(peer.fingerprint);
    });

    it('should have trusted property', function () {
      assert.isDefined(peer.trusted);
    });

    it('should have trust method', function () {
      assert.isFunction(peer.trust);
    });

    it('should be untrusted at first', function () {
      assert.equal(peer.trusted, false);
    });

    it('should refuse to encrypt for untrusted peer', function () {
      var ret = peer.encrypt('foo');
      assert.equal(ret.error, 'Peer is untrusted');
    });

    it('should refuse to encryptAndSign for untrusted peer', function () {
      var ret = peer.encryptAndSign('foo');
      assert.equal(ret.error, 'Peer is untrusted');
    });

    it('should refuse to verifyAndDecrypt for untrusted peer', function () {
      var ret = session.account.verifyAndDecrypt('foo', peer);
      assert.equal(ret.error, 'Peer is untrusted');
    });

    describe('trust()', function () {
      it('should callback without error', function (done) {
        peer.trust(function (err) {
          assert.equal(err, null);
          done();
        });
      });

      it('should set trusted property to true', function () {
        assert.equal(peer.trusted, true);
      });

      it('should add the peer\'s public key to account\'s trust state container', function (done) {
        // force the session to load from server again
        session.containers = [];

        session.getOrCreateItem(crypton.trustedPeers, function (err, trustedPeers) {
          if (err) throw err;
          var peers = trustedPeers.value;
          var savedKey = peers['notSoSmart'];
          assert.equal(typeof savedKey.trustedAt, 'number');
          assert.equal(savedKey.fingerprint, peer.fingerprint);
          done();
        });
      });

      it('should allow encrypt to run', function () {
        var ret = peer.encrypt('foo');
        assert.equal(ret.error, null);
      });

      it('should allow encryptAndSign to run', function () {
        var ret = peer.encryptAndSign('foo');
        assert.equal(ret.error, null);
      });

      it('should allow verifyAndDecrypt to run', function () {
        var ret = session.account.verifyAndDecrypt({
          ciphertext: 'foo'
        }, peer);

        // means it got past trust check
        assert.equal(ret.error, 'Cannot verify ciphertext');
      });

      it('should make peer trusted on next fetch', function (done) {
        session.getPeer('notSoSmart', function (err, newPeer) {
          if (err) throw err;
          assert.equal(newPeer.trusted, true);
          done();
        });
      });

      it('should callback with an error if peer is already trusted', function (done) {
        peer.trust(function (err) {
          assert.equal(err, 'Peer is already trusted');
          done();
        });
      });
    });
  });
});

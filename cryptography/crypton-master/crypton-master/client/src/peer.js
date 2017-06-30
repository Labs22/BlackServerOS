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

  /**!
   * # Peer(options)
   *
   * ````
   * var options = {
   *   username: 'friend' // required
   * };
   *
   * var peer = new crypton.Peer(options);
   * ````
   *
   * @param {Object} options
   */
  var Peer = crypton.Peer = function(options) {
    options = options || {};

    this.accountId = options.id;
    this.session = options.session;
    this.username = options.username;
    this.pubKey = options.pubKey;
    this.signKeyPub = options.signKeyPub;
  };

  /**!
   * ### fetch(callback)
   * Retrieve peer data from server, applying it to parent object
   *
   * Calls back with peer data and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Function} callback
   */
  Peer.prototype.fetch = function(callback) {
    if (!this.username) {
      callback('Must supply peer username');
      return;
    }

    if (!this.session) {
      callback('Must supply session to peer object');
      return;
    }

    var _this = this;
    var url = crypton.url() + '/peer/' + encodeURIComponent(this.username) + '?sid=' + crypton.sessionId;
    superagent.get(url)
      .withCredentials()
      .end(function(res) {
        if (!res.body || res.body.success !== true) {
          callback(res.body.error);
          return;
        }

        var peer = res.body.peer;
        _this.accountId = peer.accountId;
        _this.username = peer.username;
        _this.pubKey = peer.pubKey;
        _this.signKeyPub = peer.signKeyPub;

        // this may be necessary
        var point = sjcl.ecc.curves['c' + peer.pubKey.curve].fromBits(peer.pubKey.point);
        _this.pubKey = new sjcl.ecc.elGamal.publicKey(peer.pubKey.curve, point.curve, point);
        var signPoint =
          sjcl.ecc.curves['c' + peer.signKeyPub.curve].fromBits(peer.signKeyPub.point);
        _this.signKeyPub = new sjcl.ecc.ecdsa.publicKey(peer.signKeyPub.curve, signPoint.curve, signPoint);

        // calculate fingerprint for public key
        _this.fingerprint = crypton.fingerprint(_this.pubKey, _this.signKeyPub);

        callback(null, _this);
      });
  };

  /**!
   * ### encrypt(payload)
   * Encrypt `message` with peer's public key
   *
   * @param {Object} payload
   * @return {Object} ciphertext
   */
  Peer.prototype.encrypt = function(payload) {
    if (!this.trusted) {
      return {
        error: 'Peer is untrusted',
      };
    }

    // should this be async to callback with an error if there is no pubkey?
    var ciphertext = sjcl.encrypt(this.pubKey, JSON.stringify(payload), crypton.cipherOptions);
    return ciphertext;
  };

  /**!
   * ### encryptAndSign(payload)
   * Encrypt `message` with peer's public key, sign the message with own signing key
   *
   * @param {Object} payload
   * @return {Object}
   */
  Peer.prototype.encryptAndSign = function(payload) {
    if (!this.trusted) {
      return {
        error: 'Peer is untrusted',
      };
    }

    try {
      var ciphertext = sjcl.encrypt(this.pubKey, JSON.stringify(payload), crypton.cipherOptions);

      // hash the ciphertext and sign the hash:
      var hash = sjcl.hash.sha256.hash(ciphertext);
      var signature = this.session.account.signKeyPrivate.sign(hash, crypton.paranoia);
      return {
        ciphertext: JSON.parse(ciphertext),
        signature: signature,
        error: null,
      };
    } catch (ex) {
      console.error(ex);
      console.error(ex.stack);
      var err = 'Error: Could not complete encryptAndSign: ' + ex;
      return {
        ciphertext: null,
        signature: null,
        error: err,
      };
    }
  };

  /**!
   * ### sendMessage(headers, payload, callback)
   * Encrypt `headers` and `payload` and send them to peer in one logical `message`
   *
   * Calls back with message id and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} headers
   * @param {Object} payload
   */
  Peer.prototype.sendMessage = function(headers, payload, callback) {
    if (!this.session) {
      callback('Must supply session to peer object');
      return;
    }

    var message = new crypton.Message(this.session);
    message.headers = headers;
    message.payload = payload;
    message.fromAccount = this.session.accountId;
    message.toAccount = this.accountId;
    message.encrypt(this);
    message.send(callback);
  };

  /**!
   * ### trust(callback)
   * Add peer's fingerprint to internal trusted peers Item
   *
   * Calls back without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Function} callback
   */
  Peer.prototype.trust = function(callback) {
    var _this = this;

    _this.session.getOrCreateItem(crypton.trustedPeers,
      function(err, trustedPeers) {
        if (err) {
          return callback(err);
        }

        var peers = trustedPeers.value;
        if (peers[_this.username]) {
          return callback('Peer is already trusted');
        }

        peers[_this.username] = {
          trustedAt: +new Date(),
          fingerprint: _this.fingerprint,
        };

        // TODO: When this item becomes very large we might consider
        // creating items the letter of each peer's handle
        trustedPeers.save(function(err) {
          if (err) {
            return callback(err);
          }

          _this.trusted = true;
          callback(null);
        });
      });
  };

})();

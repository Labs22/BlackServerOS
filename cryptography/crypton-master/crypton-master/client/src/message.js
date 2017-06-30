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

  var Message = crypton.Message = function Message(session, raw) {
    this.session = session;
    this.headers = {};
    this.payload = {};

    raw = raw || {};
    for (var i in raw) {
      this[i] = raw[i];
    }
  };

  Message.prototype.encrypt = function(peer, callback) {
    var headersCiphertext = peer.encryptAndSign(this.headers);
    var payloadCiphertext = peer.encryptAndSign(this.payload);

    if (headersCiphertext.error || payloadCiphertext.error) {
      callback('Error encrypting headers or payload in Message.encrypt()');
      return;
    }

    this.encrypted = {
      headersCiphertext: JSON.stringify(headersCiphertext),
      payloadCiphertext: JSON.stringify(payloadCiphertext),
      fromUsername: this.session.account.username,
      toAccountId: peer.accountId,
    };

    callback && callback(null);
  };

  Message.prototype.decrypt = function(callback) {
    var _this = this;
    var headersCiphertext = JSON.parse(this.headersCiphertext);
    var payloadCiphertext = JSON.parse(this.payloadCiphertext);

    this.session.getPeer(this.fromUsername, function(err, peer) {
      if (err) {
        callback(err);
        return;
      }

      var headers = _this.session.account.verifyAndDecrypt(headersCiphertext, peer);
      var payload = _this.session.account.verifyAndDecrypt(payloadCiphertext, peer);
      if (!headers.verified || !payload.verified) {
        callback('Cannot verify headers or payload ciphertext in Message.decrypt()');
        return;
      } else if (headers.error || payload.error) {
        callback('Cannot decrypt headers or payload in Message.decrypt');
        return;
      }

      _this.headers = JSON.parse(headers.plaintext);
      _this.payload = JSON.parse(payload.plaintext);
      _this.created = new Date(_this.creationTime);
      callback(null, _this);
    });
  };

  Message.prototype.send = function(callback) {
    if (!this.encrypted) {
      return callback('You must encrypt the message to a peer before sending!');
    }

    var url = crypton.url() + '/peer?sid=' + crypton.sessionId;
    superagent.post(url)
      .send(this.encrypted)
      .withCredentials()
      .end(function(res) {
        if (!res.body || res.body.success !== true) {
          callback(res.body.error);
          return;
        }

        callback(null, res.body.messageId);
      });
  };

})();

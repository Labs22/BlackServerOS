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

  var ERRS;

  var HistoryItem =
    crypton.HistoryItem = function HistoryItem(session, rawData) {
      ERRS = crypton.errors;
      this.rawData = rawData;
      this.session = session;
      this.timelineId = rawData.timelineId;

      // fill out the rest of the properties needed at this level
      this.modTime = this.rawData.modTime;
      this.creationTime = this.rawData.creationTime;
      var record;
      if (!this.rawData.creatorUsername) {
        this.itemHistoryId = this.rawData.itemHistoryId;
        this.itemNameHmac = this.rawData.itemNameHmac;
        record = this.decryptHistoryItem(null, null); // No shortcuts for now...
        return record;
      } else {
        this.creatorUsername = this.rawData.creatorUsername;
        this.timelineId = this.rawData.timelineId;
        record = this.decryptTimelineItem(this.rawData.creatorUsername, null);
      }

      return record;
    };

  HistoryItem.prototype.decryptHistoryItem =
    function decryptHistoryItem(creator, sessionKey) {
      if (!creator) {
        this.creator = this.session.selfPeer;
      } else {
        this.creator = creator;
      }

      if (sessionKey) {
        this.sessionKey = sessionKey;
      }

      var rawData = this.rawData;
      var cipherItem = rawData.ciphertext;

      var wrappedSessionKey = JSON.parse(rawData.wrappedSessionKey);

      // check if this key is already in memory
      var _key = app.session.itemKeyLedger[wrappedSessionKey.ciphertext.kemtag];
      if (_key) {
        this.sessionKey = _key;
      }

      // XXXddahl: create 'unwrapPayload()'
      var ct = JSON.stringify(cipherItem.ciphertext);
      var hash = sjcl.hash.sha256.hash(ct);
      var verified = false;
      try {
        verified = this.creator.signKeyPub.verify(hash, cipherItem.signature);
      } catch (ex) {
        console.error(ex);
        console.error(ex.stack);
        throw new Error(ex);
      }

      // TODO: Keep a ledger of unwrapped keys so we dont have to unwrap a key for each item

      // Check for this.sessionKey, or unwrap it
      var sessionKeyResult;
      if (!this.sessionKey) {
        sessionKeyResult =
          this.session.account.verifyAndDecrypt(wrappedSessionKey, this.creator);
        if (sessionKeyResult.error) {
          return new Error(ERRS.UNWRAP_KEY_ERROR);
        }

        this.sessionKey = JSON.parse(sessionKeyResult.plaintext);
        var tag = wrappedSessionKey.ciphertext.kemtag;
        app.session.itemKeyLedger[tag] = this.sessionKey;
      }

      var decrypted;
      try {
        decrypted = sjcl.decrypt(this.sessionKey, ct, crypton.cipherOptions);
      } catch (ex) {
        console.error(ex);
        console.error(ex.stack);
        throw new Error(ERRS.DECRYPT_CIPHERTEXT_ERROR);
      }

      if (decrypted) {
        try {
          this.value = JSON.parse(decrypted);
        } catch (ex) {
          console.error(ex);
          console.error(ex.stack);

          // XXXddahl: check to see if this is an actual JSON error (malformed string, etc)
          this.value = decrypted; // Just a string, not an object
        }

        this.session.itemHistory[this.timelineId] = this;
        return this.value;
      }
    };

  HistoryItem.prototype.decryptTimelineItem =
    function decryptTimelineItem(creatorName, sessionKey) {
      var _this = this;

      // get creator
      // if creatorName is self, use selfPeer
      if (creatorName === this.session.account.username) {
        this.decryptHistoryItem(this.session.selfPeer, sessionKey);
        return;
      }

      // Handle data from actual peers...
      this.session.getPeer(creatorName, function(err, peer) {
        if (err) {
          console.error(err);
          return {
            error: err,
          };
        }

        if (!peer.trusted) {
          return console.error('Cannot decrypt data: Peer is not trusted.');
        }

        _this.decryptHistoryItem(peer, sessionKey);
      });
    };

})();

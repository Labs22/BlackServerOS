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

  var Inbox = crypton.Inbox = function Inbox(session) {
    this.session = session;
    this.rawMessages = [];
    this.messages = {};

    // XXXddahl: let's turn poll() off for now
    // this.poll();
  };

  Inbox.prototype.poll = function(callback) {
    var _this = this;
    var url = crypton.url() + '/inbox?sid=' + crypton.sessionId;
    callback = callback || function() {};

    superagent.get(url)
      .withCredentials()
      .end(function(res) {
        if (!res.body || res.body.success !== true) {
          callback(res.body.error);
          return;
        }

        // should we merge or overwrite here?
        _this.rawMessages = res.body.messages;
        _this.parseRawMessages(callback);
      });
  };

  Inbox.prototype.list = function() {
    // TODO should we poll here?
    return this.messages;
  };

  Inbox.prototype.filter = function(criteria, callback) {
    criteria = criteria || {};

    async.filter(this.messages, function(message) {
      for (var i in criteria) {
        if (message[i] !== criteria[i]) {
          return false;
        }
      }

      return true;
    }, function(messages) {

      callback(messages);
    });
  };

  Inbox.prototype.get = function(messageId, callback) {
    var _this = this;
    var cachedMessage = this.messages[messageId];

    if (cachedMessage) {
      callback(null, cachedMessage);
      return;
    }

    var url = crypton.url() + '/inbox/' + messageId + '?sid=' + crypton.sessionId;
    callback = callback || function() {};

    superagent.get(url)
      .withCredentials()
      .end(function(res) {
        if (!res.body || res.body.success !== true) {
          callback(res.body.error);
          return;
        }

        var message = new crypton.Message(_this.session, res.body.message);
        message.decrypt(function(err) {
          if (err) {
            console.error(err);
            return callback(err);
          }

          _this.messages[message.id] = message;
          callback(null, message);
        });
      });
  };

  Inbox.prototype.delete = function(id, callback) {
    var chunk = {
      type: 'deleteMessage',
      messageId: id,
    };

    // TODO handle errs
    var tx = new crypton.Transaction(this.session, function(err) {
      tx.save(chunk, function(err) {
        tx.commit(function(err) {
          callback();
        });
      });
    });
  };

  Inbox.prototype.getAllMetadata = function(callback) {
    var url = crypton.url() + '/inbox-metadata?sid=' + crypton.sessionId;
    callback = callback || function() {};

    superagent.get(url)
      .withCredentials()
      .end(function(res) {
        if (!res.body || res.body.success !== true) {
          callback(res.body.error);
          return;
        }

        callback(null, res.body.metadata);
        return;
      });
  };

  Inbox.prototype.clear = function(callback) {
    // start + commit tx
    var chunk = {
      type: 'clearInbox',
    };

    // TODO handle errs
    var tx = new crypton.Transaction(this.session, function(err) {
      tx.save(chunk, function(err) {
        tx.commit(function(err) {
          callback();
        });
      });
    });
  };

  Inbox.prototype.parseRawMessages = function(callback) {
    var _this = this;
    async.each(this.rawMessages, function(rawMessage, callback) {
      var message = new crypton.Message(_this.session, rawMessage);
      message.decrypt(function(err) {
        _this.messages[message.messageId] = message;
        callback();
      });
    }, function(err) {

      if (callback) {
        callback(null, _this.messages);
      }
    });
  };

})();

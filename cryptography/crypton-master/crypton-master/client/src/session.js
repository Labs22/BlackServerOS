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

  /**!
   * # Session(id)
   *
   * ````
   * var session = new crypton.Session(id);
   * ````
   *
   * @param {Number} id
   */
  var Session = crypton.Session = function(id) {
    ERRS = crypton.errors;
    this.id = id;
    this.peers = {};
    this.events = {};
    this.containers = [];
    this.items = {};
    this.itemHistory = {};
    this.itemKeyLedger = {};
    this.timeline = [];
    this.inbox = new crypton.Inbox(this);

    var _this = this;

    var joinServerParameters = {
      token: crypton.sessionId,
    };

    this.socket = io.connect(crypton.url(), {
      query: 'joinServerParameters=' +
        JSON.stringify(joinServerParameters),
      reconnection: true,
      reconnectionDelay: 5000,
    });

    // watch for incoming Inbox messages
    this.socket.on('message', function(data) {
      _this.inbox.get(data.messageId, function(err, message) {
        _this.emit('message', message);
      });
    });

    // watch for container update notifications
    this.socket.on('containerUpdate', function(containerNameHmac) {
      // if any of the cached containers match the HMAC
      // in the notification, sync the container and
      // call the listener if one has been set
      for (var i = 0; i < _this.containers.length; i++) {
        var container = _this.containers[i];
        var temporaryHmac = container.containerNameHmac || container.getPublicName();

        if (crypton.constEqual(temporaryHmac, containerNameHmac)) {
          container.sync(function(err) {
            if (container._listener) {
              container._listener();
            }
          });

          break;
        }
      }
    });

    // watch for Item update notifications
    this.socket.on('itemUpdate', function(itemObj) {
      if (!itemObj.itemNameHmac || !itemObj.creator || !itemObj.toUsername) {
        console.error(ERRS.ARG_MISSING);
        throw new Error(ERRS.ARG_MISSING);
      }

      // if any of the cached items match the HMAC
      // in the notification, sync the items and
      // call the listener if one has been set
      if (_this.items[itemObj.itemNameHmac]) {

        _this.items[itemObj.itemNameHmac].sync(function(err) {
          if (err) {
            return console.error(err);
          }

          try {
            _this.events.onSharedItemSync(_this.items[itemObj.itemNameHmac]);
          } catch (ex) {
            console.warn(ex);
          }

          if (_this.items[itemObj.itemNameHmac]._listener) {
            _this.items[itemObj.itemNameHmac]._listener(err);
          }
        });
      } else {
        // load item!
        // get the peer first:
        _this.getPeer(itemObj.creator, function(err, peer) {
          if (err) {
            console.error(err);
            console.error('Cannot load item: creator peer cannot be found');
            return;
          }

          // XXXddahl: Make sure you trust this peer before loading the item
          //           Perhaps we check this inside the Item constructor?
          var itemCallback = function _itemCallback(err, item) {
            if (err) {
              console.error(err);
              return;
            }

            _this.items[itemObj.itemNameHmac] = item;
            try {
              _this.events.onSharedItemSync(item);
            } catch (ex) {
              console.warn(ex);
            }
          };

          var item =
            new crypton.Item(null, null, _this, peer,
              itemCallback, itemObj.itemNameHmac);
        });
      }
    });
  };

  /**!
   * ### removeItem(itemNameHmac, callback)
   * Remove/delete Item with given 'itemNameHmac',
   * both from local cache & server
   *
   * Calls back with success boolean and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} itemNameHmac
   * @param {Function} callback
   */
  Session.prototype.removeItem = function removeItem(itemNameHmac, callback) {
    var _this = this;
    for (var name in this.items) {
      if (this.items[name].nameHmac === itemNameHmac) {
        this.items[name].remove(function(err) {
          if (err) {
            console.error(err);
            callback(err);
            return;
          }

          if (_this.items[name].deleted) {
            delete _this.items[name];
            callback(null);
          }
        });
      }
    }
  };

  /**!
   * ### getOrCreateItem(itemName, callback)
   * Create or Retrieve Item with given platintext `itemName`,
   * either from local cache or server
   *
   * This method is for use by the creator of the item.
   * Use 'session.getSharedItem' for items shared by the creator
   *
   * Calls back with Item and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} itemName
   * @param {Function} callback
   */
  Session.prototype.getOrCreateItem =
    function getOrCreateItem(itemName, callback) {

      if (!itemName) {
        return callback('itemName is required');
      }

      if (!callback) {
        throw new Error('Missing required callback argument');
      }

      // Get cached item if exists
      // XXXddahl: check server for more recent item?
      // We need another server API like /itemupdated/<itemHmacName> which returns
      // the timestamp of the last update
      if (this.items[itemName]) {
        callback(null, this.items[itemName]);
        return;
      }

      var creator = this.selfPeer;
      var item = new crypton.Item(itemName, null, this, creator,
        function getItemCallback(err, item) {
          if (err) {
            console.error(err);
            return callback(err);
          }

          callback(null, item);
        });
    };

  /**!
   * ### getSharedItem(itemNameHmac, peer, callback)
   * Retrieve shared Item with given itemNameHmac,
   * either from local cache or server
   *
   * Calls back with Item and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} itemNameHmac
   * @param {Object} peer
   * @param {Function} callback
   */
  Session.prototype.getSharedItem =
    function getSharedItem(itemNameHmac, peer, callback) {
      // TODO:  Does not check for cached item or server having a fresher Item
      if (!itemNameHmac) {
        return callback(ERRS.ARG_MISSING);
      }

      if (!callback) {
        throw new Error(ERRS.ARG_MISSING_CALLBACK);
      }

      function getItemCallback(err, item) {
        if (err) {
          console.error(err);
          return callback(err);
        }

        callback(null, item);
      }

      new crypton.Item(null, null, this, peer, getItemCallback, itemNameHmac);
    };

  Session.prototype.__defineGetter__('selfPeer', function() {
    if (this._selfPeer) {
      return this._selfPeer;
    }

    this._selfPeer = this.createSelfPeer();
    return this._selfPeer;
  });

  /**!
   * ### createSelfPeer()
   * returns a 'selfPeer' object which is needed for any kind of
   * self-signing, encryption or verification
   *
   */
  Session.prototype.createSelfPeer = function() {
    var selfPeer = new crypton.Peer({
      session: this,
      pubKey: this.account.pubKey,
      signKeyPub: this.account.signKeyPub,
      signKeyPrivate: this.account.signKeyPrivate,
      username: this.account.username,
    });
    selfPeer.trusted = true;
    return selfPeer;
  };

  /**!
   * ### getItemHistory()
   * returns a row of items the user created
   *
   * Calls back with ItemHistory Array and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} options // e.g.: {lastItemRead: 12,offset: 350,limit: 20 }
   * @param {Function} callback
   */
  Session.prototype.getItemHistory =
    function getItemHistory(options, callback) {
      if (typeof options !== 'object') {
        return callback(ERRS.ARG_MISSING_OBJECT);
      }

      if (typeof callback !== 'function') {
        return callback(ERRS.ARG_MISSING_CALLBACK);
      }

      var lastItemRead = options.lastItemRead; // item_history_id
      var offset = options.offset;
      var limit = options.limit;

      if (typeof parseInt(lastItemRead) !== 'number') {
        lastItemRead = 0;
      }

      if (typeof parseInt(offset) !== 'number') {
        offset = 0;
      }

      if (typeof parseInt(limit) !== 'number') {
        limit = 10; // default MAX of 10 - for now
      }

      var _this = this;
      var url = crypton.url() + '/itemhistory/' + '?sid=' + crypton.sessionId +
        '&historyid=' + lastItemRead + // item_history_id
        '&offset=' + offset +
        '&limit=' + limit;

      superagent.get(url)
        .withCredentials()
        .end(function(res) {
          if (!res.body || res.body.success !== true) {
            console.error(res.body);
            return callback('Cannot get item history');
          }

          // expand all item_history rows into actual items
          var rows = res.body.rawData;
          var history = [];

          // XXXddahl: use async() ?
          for (var i = 0; i < rows.length; i++) {
            var timelineId = rows[i].timelineId;
            if (_this.itemHistory[timelineId]) {
              // Let's not re-decrypt something _this is most likely in our feed
              continue;
            }

            var hitem = new crypton.HistoryItem(_this, rows[i]);
            history.push(hitem);
          }

          callback(null, history);
        });
    };

  /**!
   * ### getTimeline()
   * returns a row of Timeline items
   *
   * Calls back with ItemHistory Array and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} options // e.g.: {lastItemRead: 12,offset: 350,limit: 20 }
   * @param {Function} callback
   */
  Session.prototype.getTimeline =
    function getTimeline(options, callback) {
      if (typeof options !== 'object') {
        return callback(ERRS.ARG_MISSING_OBJECT);
      }

      if (typeof callback !== 'function') {
        return callback(ERRS.ARG_MISSING_CALLBACK);
      }

      var lastItemRead = options.lastItemRead; // item_history_id
      var offset = options.offset;
      var limit = options.limit;
      var direction = options.direction;

      if (typeof parseInt(lastItemRead) !== 'number') {
        lastItemRead = 0;
      }

      if (typeof parseInt(offset) !== 'number') {
        offset = 0;
      }

      if (typeof parseInt(limit) !== 'number') {
        limit = 10; // default MAX of 10 - for now
      }

      var _this = this;
      var url = crypton.url() + '/timeline/' + '?sid=' + crypton.sessionId +
        '&timelineid=' + lastItemRead + // timeline_id
        '&offset=' + offset +
        '&limit=' + limit +
        '&direction=' + direction;

      superagent.get(url)
        .withCredentials()
        .end(function(res) {
          if (!res.body || res.body.success !== true) {
            console.error(res.body);
            return callback('Cannot get timeline');
          }

          // expand all item_history rows into actual items
          var rows = res.body.rawData;
          var history = [];

          // XXXddahl: use async() ?
          for (var i = 0; i < rows.length; i++) {
            var hitem = new crypton.HistoryItem(_this, rows[i]);
            history.push(hitem);
          }

          callback(null, history);
        });
    };

  /**!
   * ### getLatestTimeline()
   * returns latest Timeline items
   *
   * Calls back with ItemHistory Array and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} options // e.g.: { limit: 20 }
   * @param {Function} callback
   */
  Session.prototype.getLatestTimeline =
    function getLatestTimeline(options, callback) {
      if (typeof options !== 'object') {
        return callback(ERRS.ARG_MISSING_OBJECT);
      }

      if (typeof callback !== 'function') {
        return callback(ERRS.ARG_MISSING_CALLBACK);
      }

      var limit = options.limit;

      if (typeof parseInt(limit) !== 'number') {
        limit = 10; // default MAX of 10 - for now
      }

      var _this = this;
      var url = crypton.url() + '/timeline-latest/' + '?sid=' + crypton.sessionId +
        '&limit=' + limit;

      superagent.get(url)
        .withCredentials()
        .end(function(res) {
          if (!res.body || res.body.success !== true) {
            console.error(res.body);
            return callback('Cannot get timeline');
          }

          // expand all item_history rows into actual items
          var rows = res.body.rawData;
          var history = [];
          for (var i = 0; i < rows.length; i++) {
            var hitem = new crypton.HistoryItem(_this, rows[i]);

            // HistoryItem will return null if the item is
            // supposed to be ignored by the Timeline code
            if (hitem === null) {
              continue;
            }

            history.push(hitem);
          }

          history.reverse();

          callback(null, history);
        });
    };

  /**!
   * ### getTimelineBefore()
   * returns Timeline items before ID
   *
   * Calls back with ItemHistory Array and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} options // e.g.: { limit: 20, beforeId: 12345 }
   * @param {Function} callback
   */
  Session.prototype.getTimelineBefore =
    function getTimelineBefore(options, callback) {
      if (typeof options !== 'object') {
        return callback(ERRS.ARG_MISSING_OBJECT);
      }

      if (typeof callback !== 'function') {
        return callback(ERRS.ARG_MISSING_CALLBACK);
      }

      var limit = options.limit;

      if (typeof parseInt(limit) !== 'number') {
        limit = 10; // default MAX of 10 - for now
      }

      var beforeId = options.beforeId;

      if (typeof parseInt(beforeId) !== 'number') {
        console.error('\'beforeId\' property is missing from the options argument');
        return callback(ERRS.ARG_MISSING);
      }

      var _this = this;
      var url = crypton.url() +
        '/timeline-before/' +
        '?sid=' +
        crypton.sessionId +
        '&limit=' + limit +
        '&beforeId=' + beforeId;

      superagent.get(url)
        .withCredentials()
        .end(function(res) {
          if (!res.body || res.body.success !== true) {
            console.error(res.body);
            return callback('Cannot get timeline');
          }

          // expand all item_history rows into actual items
          var rows = res.body.rawData;
          var history = [];
          for (var i = 0; i < rows.length; i++) {
            var hitem = new crypton.HistoryItem(_this, rows[i]);
            history.push(hitem);
          }

          callback(null, history);
        });
    };

  /**!
   * ### getTimelineAfter()
   * returns Timeline items after ID
   *
   * Calls back with ItemHistory Array and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} options // e.g.: { limit: 20, afterId: 12345 }
   * @param {Function} callback
   */
  Session.prototype.getTimelineAfter =
    function getTimelineAfter(options, callback) {
      if (typeof options !== 'object') {
        return callback(ERRS.ARG_MISSING_OBJECT);
      }

      if (typeof callback !== 'function') {
        return callback(ERRS.ARG_MISSING_CALLBACK);
      }

      var limit = options.limit;
      if (typeof parseInt(limit) !== 'number') {
        limit = 10; // default MAX of 10 - for now
      }

      var afterId = options.afterId;
      if (typeof parseInt(afterId) !== 'number') {
        console.error('\'afterId\' property is missing from the options argument');
        return callback(ERRS.ARG_MISSING);
      }

      var _this = this;

      var url = crypton.url() + '/timeline-after/' + '?sid=' + crypton.sessionId +
        '&limit=' + limit +
        '&afterId=' + afterId;

      superagent.get(url)
        .withCredentials()
        .end(function(res) {
          if (!res.body || res.body.success !== true) {
            console.error(res.body);
            return callback('Cannot get timeline');
          }

          // expand all item_history rows into actual items
          var rows = res.body.rawData;
          var history = [];
          for (var i = 0; i < rows.length; i++) {
            var hitem = new crypton.HistoryItem(_this, rows[i]);
            history.push(hitem);
          }

          history.reverse();
          callback(null, history);
        });
    };

  // =================== Containers ===================== //

  /**!
   * ### load(containerName, callback)
   * Retieve container with given platintext `containerName`,
   * either from local cache or server
   *
   * Calls back with container and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} containerName
   * @param {Function} callback
   */
  Session.prototype.load = function(containerName, callback) {
    console.warn('CONTAINERS ARE DEPRECATED, use the "Items" API');

    // check for a locally stored container
    for (var i = 0; i < this.containers.length; i++) {
      if (crypton.constEqual(this.containers[i].name, containerName)) {
        callback(null, this.containers[i]);
        return;
      }
    }

    // check for a container on the server
    var _this = this;
    this.getContainer(containerName, function(err, container) {
      if (err) {
        callback(err);
        return;
      }

      _this.containers.push(container);
      callback(null, container);
    });
  };

  /**!
   * ### loadWithHmac(containerNameHmac, callback)
   * Retieve container with given `containerNameHmac`,
   * either from local cache or server
   *
   * Calls back with container and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} containerNameHmac
   * @param {Function} callback
   */
  Session.prototype.loadWithHmac = function(containerNameHmac, peer, callback) {
    console.warn('CONTAINERS ARE DEPRECATED, use the "Items" API');

    // check for a locally stored container
    for (var i = 0; i < this.containers.length; i++) {
      if (crypton.constEqual(this.containers[i].nameHmac, containerNameHmac)) {
        callback(null, this.containers[i]);
        return;
      }
    }

    // check for a container on the server
    var _this = this;
    this.getContainerWithHmac(containerNameHmac, peer, function(err, container) {
      if (err) {
        callback(err);
        return;
      }

      _this.containers.push(container);
      callback(null, container);
    });
  };

  /**!
   * ### create(containerName, callback)
   * Create container with given platintext `containerName`,
   * save it to server
   *
   * Calls back with container and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} containerName
   * @param {Function} callback
   */
  Session.prototype.create = function(containerName, callback) {
    console.warn('CONTAINERS ARE DEPRECATED, use the "Items" API');

    for (var i in this.containers) {
      if (crypton.constEqual(this.containers[i].name, containerName)) {
        callback('Container already exists');
        return;
      }
    }

    var selfPeer = new crypton.Peer({
      session: this,
      pubKey: this.account.pubKey,
      signKeyPub: this.account.signKeyPub,
    });
    selfPeer.trusted = true;

    var sessionKey = crypton.randomBytes(32);
    var sessionKeyCiphertext = selfPeer.encryptAndSign(sessionKey);

    if (sessionKeyCiphertext.error) {
      return callback(sessionKeyCiphertext.error);
    }

    delete sessionKeyCiphertext.error;

    // TODO is signing the sessionKey even necessary if we're
    // signing the sessionKeyShare? what could the container
    // creator attack by wrapping a different sessionKey?
    var signature = 'hello';
    var containerNameHmac = new sjcl.misc.hmac(this.account.containerNameHmacKey);
    containerNameHmac = sjcl.codec.hex.fromBits(containerNameHmac.encrypt(containerName));

    // TODO why is a session object generating container payloads? creating the
    // initial container state should be done in container.js
    var rawPayloadCiphertext = sjcl.encrypt(sessionKey, JSON.stringify({
      recordIndex: 0,
      delta: {},
    }), crypton.cipherOptions);

    var payloadCiphertextHash = sjcl.hash.sha256.hash(JSON.stringify(rawPayloadCiphertext));
    var payloadSignature = this.account.signKeyPrivate.sign(payloadCiphertextHash, crypton.paranoia);

    var payloadCiphertext = {
      ciphertext: rawPayloadCiphertext,
      signature: payloadSignature,
    };

    var _this = this;

    new crypton.Transaction(this, function(err, tx) {
      var chunks = [
        { type: 'addContainer',
          containerNameHmac: containerNameHmac,
        },
        { type: 'addContainerSessionKey',
          containerNameHmac: containerNameHmac,
          signature: signature,
        },
        { type: 'addContainerSessionKeyShare',
          toAccount: _this.account.username,
          containerNameHmac: containerNameHmac,
          sessionKeyCiphertext: sessionKeyCiphertext,
        },
        { type: 'addContainerRecord',
          containerNameHmac: containerNameHmac,
          payloadCiphertext: payloadCiphertext,
        },
      ];

      async.eachSeries(chunks, function(chunk, callback2) {
        tx.save(chunk, callback2);
      }, function(err) {
        if (err) {
          return tx.abort(function() {
            callback(err);
          });
        }

        tx.commit(function() {
          var container = new crypton.Container(_this);
          container.name = containerName;
          container.sessionKey = sessionKey;
          _this.containers.push(container);
          callback(null, container);
        });
      });
    });
  };

  /**!
   * ### deleteContainer(containerName, callback)
   * Request the server to delete all records and keys
   * belonging to `containerName`
   *
   * Calls back without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} containerName
   * @param {Function} callback
   */
  Session.prototype.deleteContainer = function(containerName, callback) {
    console.warn('CONTAINERS ARE DEPRECATED, use the "Items" API');

    var _this = this;
    var containerNameHmac = new sjcl.misc.hmac(this.account.containerNameHmacKey);
    containerNameHmac = sjcl.codec.hex.fromBits(containerNameHmac.encrypt(containerName));

    new crypton.Transaction(this, function(err, tx) {
      var chunk = {
        type: 'deleteContainer',
        containerNameHmac: containerNameHmac,
      };

      tx.save(chunk, function(err) {
        if (err) {
          return callback(err);
        }

        tx.commit(function(err) {
          if (err) {
            return callback(err);
          }

          // remove from cache
          for (var i = 0; i < _this.containers.length; i++) {
            if (crypton.constEqual(_this.containers[i].name, containerName)) {
              _this.containers.splice(i, 1);
              break;
            }
          }

          callback(null);
        });
      });
    });
  };

  /**!
   * ### getContainer(containerName, callback)
   * Retrieve container with given platintext `containerName`
   * specifically from the server
   *
   * Calls back with container and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} containerName
   * @param {Function} callback
   */
  Session.prototype.getContainer = function(containerName, callback) {
    console.warn('CONTAINERS ARE DEPRECATED, use the "Items" API');

    var container = new crypton.Container(this);
    container.name = containerName;
    container.sync(function(err) {
      callback(err, container);
    });
  };

  /**!
   * ### getContainerWithHmac(containerNameHmac, callback)
   * Retrieve container with given `containerNameHmac`
   * specifically from the server
   *
   * Calls back with container and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} containerNameHmac
   * @param {Function} callback
   */
  Session.prototype.getContainerWithHmac = function(containerNameHmac, peer, callback) {
    console.warn('CONTAINERS ARE DEPRECATED, use the "Items" API');

    var container = new crypton.Container(this);
    container.nameHmac = containerNameHmac;
    container.peer = peer;
    container.sync(function(err) {
      callback(err, container);
    });
  };

  /**!
   * ### getPeer(containerName, callback)
   * Retrieve a peer object from the database for given `username`
   *
   * Calls back with peer and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} username
   * @param {Function} callback
   */
  Session.prototype.getPeer = function(username, callback) {
    if (this.peers[username]) {
      return callback(null, this.peers[username]);
    }

    var _this = this;
    var peer = new crypton.Peer();
    peer.username = username;
    peer.session = this;

    peer.fetch(function(err, peer) {
      if (err) {
        return callback(err);
      }

      _this.getOrCreateItem(crypton.trustedPeers, function(err, trustedPeers) {
        if (err) {
          return callback(err);
        }

        // if the peer has previously been trusted,
        // we should check the saved fingerprint against
        // what the server has given us
        var peers = trustedPeers.value;

        if (!peers[username]) {
          peer.trusted = false;
        } else {
          var savedFingerprint = peers[username].fingerprint;
          if (!crypton.constEqual(savedFingerprint, peer.fingerprint)) {
            return callback('Server has provided malformed peer', peer);
          }

          peer.trusted = true;
        }

        _this.peers[username] = peer;
        callback(null, peer);
      });
    });
  };

  /**!
   * ### on(eventName, listener)
   * Set `listener` to be called anytime `eventName` is emitted
   *
   * @param {String} eventName
   * @param {Function} listener
   */
  Session.prototype.on = function(eventName, listener) {
    // TODO allow multiple listeners
    this.events[eventName] = listener;
  };

  /**!
   * ### emit(eventName, data)
   * Call listener for `eventName`, passing it `data` as an argument
   *
   * @param {String} eventName
   * @param {Object} data
   */
  Session.prototype.emit = function(eventName, data) {
    // TODO allow multiple listeners
    this.events[eventName] && this.events[eventName](data);
  };

})();

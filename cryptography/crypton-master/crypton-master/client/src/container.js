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
   * # Container(session)
   *
   * ````
   * var container = new crypton.Container(session);
   * ````
   *
   * @param {Object} session
   */
  var Container = crypton.Container = function(session) {
    this.keys = {};
    this.session = session;
    this.recordCount = 1;
    this.recordIndex = 0;
    this.versions = {};

    //this.version = +new Date();
    this.version = 0;
    this.name = null;
  };

  /**!
   * ### add(key, callback)
   * Add given `key` to the container
   *
   * Calls back without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} key
   * @param {Function} callback
   */
  Container.prototype.add = function(key, callback) {
    if (this.keys[key]) {
      callback('Key already exists');
      return;
    }

    this.keys[key] = {};
    callback();
  };

  /**!
   * ### get(key, callback)
   * Retrieve value for given `key`
   *
   * Calls back with `value` and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {String} key
   * @param {Function} callback
   */
  Container.prototype.get = function(key, callback) {
    if (!this.keys[key]) {
      callback('Key does not exist');
      return;
    }

    callback(null, this.keys[key]);
  };

  /**!
   * ### save(callback, options)
   * Get difference of container since last save (a record),
   * encrypt the record, and send it to the server to be saved
   *
   * Calls back without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Function} callback
   * @param {Object} options (optional)
   */
  Container.prototype.save = function(callback, options) {
    var _this = this;

    this.getDiff(function(err, diff) {
      if (!diff) {
        callback('Container has not changed');
        return;
      }

      var payload = {
        recordIndex: _this.recordCount,
        delta: diff,
      };

      var now = +new Date();
      _this.versions[now] = JSON.parse(JSON.stringify(_this.keys));
      _this.version = now;
      _this.recordCount++;

      var rawPayloadCiphertext = sjcl.encrypt(_this.sessionKey, JSON.stringify(payload), crypton.cipherOptions);
      var payloadCiphertextHash = sjcl.hash.sha256.hash(JSON.stringify(rawPayloadCiphertext));
      var payloadSignature = _this.session.account.signKeyPrivate.sign(payloadCiphertextHash, crypton.paranoia);

      var payloadCiphertext = {
        ciphertext: rawPayloadCiphertext,
        signature: payloadSignature,
      };

      var chunk = {
        type: 'addContainerRecord',
        containerNameHmac: _this.getPublicName(),
        payloadCiphertext: payloadCiphertext,
      };

      // if we aren't saving it, we're probably testing
      // to see if the transaction chunk was generated correctly
      if (options && options.save === false) {
        callback(null, chunk);
        return;
      }

      // TODO handle errs
      var tx = new crypton.Transaction(_this.session, function(err) {
        if (err) {
          console.error(err);
        }

        tx.save(chunk, function(err) {
          if (err) {
            console.error(err);
          }

          tx.commit(function(err) {
            if (err) {
              console.error(err);
            }

            callback(err);
          });
        });
      });
    });
  };

  /**!
   * ### getDiff(callback, options)
   * Compute difference of container since last save
   *
   * Calls back with diff object and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Function} callback
   */
  Container.prototype.getDiff = function(callback) {
    var last = this.latestVersion();
    var old = this.versions[last] || {};
    callback(null, crypton.diff.create(old, this.keys));
  };

  /**!
   * ### getVersions()
   * Return a list of known save point timestamps
   *
   * @return {Array} timestamps
   */
  Container.prototype.getVersions = function() {
    return Object.keys(this.versions);
  };

  /**!
   * ### getVersion(version)
   * Return full state of container at given `timestamp`
   *
   * @param {Number} timestamp
   * @return {Object} version
   */
  Container.prototype.getVersion = function(timestamp) {
    return this.versions[timestamp];
  };

  /**!
   * ### getVersion()
   * Return last known save point timestamp
   *
   * @return {Number} version
   */
  Container.prototype.latestVersion = function() {
    var versions = this.getVersions();

    if (!versions.length) {
      return this.version;
    } else {
      return Math.max.apply(Math, versions);
    }
  };

  /**!
   * ### getPublicName()
   * Compute the HMAC for the given name of the container
   *
   * @return {String} hmac
   */
  Container.prototype.getPublicName = function() {
    if (this.nameHmac) {
      return this.nameHmac;
    }

    var hmac = new sjcl.misc.hmac(this.session.account.containerNameHmacKey);
    var containerNameHmac = hmac.encrypt(this.name);
    this.nameHmac = sjcl.codec.hex.fromBits(containerNameHmac);
    return this.nameHmac;
  };

  /**!
   * ### getHistory()
   * Ask the server for all state records
   *
   * Calls back with diff object and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Function} callback
   */
  Container.prototype.getHistory = function(callback) {
    var containerNameHmac = this.getPublicName();
    var currentVersion = this.latestVersion();

    var url = crypton.url() + '/container/' + containerNameHmac + '?after=' + (currentVersion + 1) + '&sid=' + crypton.sessionId;
    console.log('getHistory', url);
    superagent.get(url)
      .withCredentials()
      .end(function(res) {
        if (!res.body || res.body.success !== true) {
          callback(res.body.error);
          return;
        }

        callback(null, res.body.records);
      });
  };

  /**!
   * ### parseHistory(records, callback)
   * Loop through given `records`, decrypt them,
   * and build object state from decrypted diff objects
   *
   * Calls back with full container state,
   * history versions, last record index,
   * and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Array} records
   * @param {Function} callback
   */
  Container.prototype.parseHistory = function(records, callback) {
    var _this = this;
    var keys = _this.keys || {};
    var versions = _this.versions || {};

    var recordIndex = _this.recordIndex + 1;

    async.eachSeries(records, function(rawRecord, callback) {
      _this.decryptRecord(recordIndex, rawRecord, function(err, record) {
        if (err) {
          return callback(err);
        }

        // TODO put in worker
        keys = crypton.diff.apply(record.delta, keys);

        // stringify and parse keys to ensure clean clone
        versions[record.time] = JSON.parse(JSON.stringify(keys));

        callback(null);
      });
    }, function(err) {

      if (err) {
        console.log('Hit error parsing container history');
        console.log(_this);
        console.log(err);

        return callback(err);
      }

      _this.recordIndex = recordIndex;
      callback(null, keys, versions, recordIndex);
    });
  };

  /**!
   * ### decryptRecord(recordIndex, record, callback)
   * Decrypt record ciphertext with session key,
   * verify record index
   *
   * Calls back with object containing timestamp and delta
   * and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} recordIndex
   * @param {Object} record
   * @param {Object} callback
   */
  Container.prototype.decryptRecord = function(recordIndex, record, callback) {
    if (!this.sessionKey) {
      this.decryptKey(record);
    }

    var parsedRecord;
    try {
      parsedRecord = JSON.parse(record.payloadCiphertext);
    } catch (e) {}

    if (!parsedRecord) {
      return callback('Could not parse record JSON');
    }

    // we can't just send the peer object or its signKeyPub
    // here because of circular JSON when dealing with workers.
    // we'll have to reconstruct the signkey on the other end.
    // better to be explicit anyway!
    var options = {
      sessionKey: this.sessionKey,
      expectedRecordIndex: recordIndex,
      record: record.payloadCiphertext,
      creationTime: record.creationTime,
      peerSignKeyPubSerialized: (
        this.peer && this.peer.signKeyPub || this.session.account.signKeyPub
      ).serialize(),
    };

    crypton.work.decryptRecord(options, callback);
  };

  /**!
   * ### decryptKey(record)
   * Extract and decrypt the container's keys from a given record
   *
   * @param {Object} record
   */
  Container.prototype.decryptKey = function(record) {
    var peer = this.peer || this.session.account;
    var sessionKeyRaw = this.session.account.verifyAndDecrypt(JSON.parse(record.sessionKeyCiphertext), peer);

    if (sessionKeyRaw.error) {
      throw new Error(sessionKeyRaw.error);
    }

    if (!sessionKeyRaw.verified) {
      throw new Error('Container session key signature mismatch');
    }

    this.sessionKey = JSON.parse(sessionKeyRaw.plaintext);
  };

  /**!
   * ### sync(callback)
   * Retrieve history, decrypt it, and update
   * container object with new state
   *
   * Calls back without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Function} callback
   */
  Container.prototype.sync = function(callback) {
    var _this = this;
    this.getHistory(function(err, records) {
      if (err) {
        callback(err);
        return;
      }

      _this.parseHistory(records, function(err, keys, versions, recordIndexAfter) {
        _this.keys = keys;
        _this.versions = versions;
        _this.version = Math.max.apply(Math, Object.keys(versions));
        _this.recordCount = _this.recordCount + versions.count;

        // TODO verify recordIndexAfter == recordCount?

        callback(err);
      });
    });
  };

  /**!
   * ### share(peer, callback)
   * Encrypt the container's sessionKey with peer's
   * public key, commit new addContainerSessionKey chunk,
   * and send a message to the peer informing them
   *
   * Calls back without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Function} callback
   */
  Container.prototype.share = function(peer, callback) {
    if (!this.sessionKey) {
      return callback('Container must be initialized to share');
    }

    // get the containerNameHmac
    // TODO this won't work if you aren't original sharer
    // because you won't have the original containerNameHmacKey.
    // we will have to mark containers as origin or remote
    var containerNameHmac = this.getPublicName();

    // encrypt sessionKey to peer's pubKey
    var sessionKeyCiphertext = peer.encryptAndSign(this.sessionKey);

    if (sessionKeyCiphertext.error) {
      return callback(sessionKeyCiphertext.error);
    }

    delete sessionKeyCiphertext.error;

    // create new addContainerSessionKeyShare chunk
    var _this = this;
    new crypton.Transaction(this, function(err, tx) {
      var chunk = {
        type: 'addContainerSessionKeyShare',
        toAccount: peer.username,
        containerNameHmac: containerNameHmac,
        sessionKeyCiphertext: sessionKeyCiphertext,
      };

      tx.save(chunk, function(err) {
        if (err) {
          return callback(err);
        }

        tx.commit(function(err) {
          if (err) {
            return callback(err);
          }

          // send a message informing peer
          // TODO we will have to add the ability to mark which application
          // this container belongs to, otherwise an application on the same
          // crypton server may incorrectly act upon this message
          peer.sendMessage({
            type: 'internal',
            action: 'containerShare',
          }, {
            fromUsername: _this.session.account.username,
            containerNameHmac: containerNameHmac,
          }, function(err) {
            callback(err);
          });
        });
      });
    });
  };

  /**!
   * ### watch(listener)
   * Attach a listener to the container
   * which is called if it is written to by a peer
   *
   * This is called after the container is synced
   *
   * @param {Function} callback
   */
  Container.prototype.watch = function(listener) {
    this._listener = listener;
  };

  /**!
   * ### unwatch()
   * Remove an attached listener
   */
  Container.prototype.unwatch = function() {
    delete this._listener;
  };

})();

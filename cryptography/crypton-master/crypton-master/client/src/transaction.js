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
   * # Transaction(session, callback)
   *
   * ````
   * var tx = new crypton.Transaction(session, function (err, tx) {
   *   // your code
   * });
   * ````
   *
   * @param {Object} session
   * @param {Function} callback
   */
  var Transaction = crypton.Transaction = function(session, callback) {
    var _this = this;
    this.session = session;
    this.types = [
      'addAccount',
      'setBaseKeyring',
      'addContainer',
      'deleteContainer',
      'addContainerSessionKey',
      'addContainerSessionKeyShare',
      'addContainerRecord',
      'addMessage',
      'deleteMessage',
    ];

    // necessary for testing
    if (!this.session) {
      callback && callback(null, this);
      return;
    }

    this.create(function(err, id) {
      if (err) {
        console.log(err);
        callback(err);
        return;
      }

      _this.id = id;

      callback && callback(null, _this);
    });
  };

  /**!
   * ### create(callback)
   * Ask the server for a new transaction id
   *
   * Calls back with transaction id and without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Function} callback
   */
  Transaction.prototype.create = function(callback) {
    var url = crypton.url() + '/transaction/create' + '?sid=' + crypton.sessionId;
    superagent.post(url)
      .withCredentials()
      .end(function(res) {
        if (!res.body || res.body.success !== true) {
          callback(res.body.error);
          return;
        }

        callback(null, res.body.id);
      });
  };

  /**!
   * ### save(chunk, ..., callback)
   * Save set of chunks to the server
   *
   * Calls back without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} chunk
   * @param {Function} callback
   */
  Transaction.prototype.save = function() {
    this.verify();
    var _this = this;
    var args = Array.prototype.slice.call(arguments);
    var callback = args.pop();

    if (typeof callback !== 'function') {
      args.push(callback);
      callback = function() {};
    }

    async.each(args, function(chunk, cb) {
      // TODO check the type of the object
      if (typeof chunk === 'function') {
        cb();
        return;
      }

      _this.saveChunk(chunk, cb);
    }, function(err) {
      callback(err);
    });
  };

  /**!
   * ### saveChunk(chunk, callback)
   * Save single chunk to the server
   *
   * Calls back without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Object} chunk
   * @param {Function} callback
   */
  Transaction.prototype.saveChunk = function(chunk, callback) {
    this.verify();
    this.verifyChunk(chunk);
    var url = crypton.url() + '/transaction/' + this.id + '?sid=' + crypton.sessionId;

    superagent.post(url)
      .withCredentials()
      .send(chunk)
      .end(function(res) {
        if (!res.body || res.body.success !== true) {
          callback(res.body.error);
          return;
        }

        callback();
      });
  };

  /**!
   * ### commit(callback)
   * Ask the server to queue the transaction for committal
   *
   * Calls back without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Function} callback
   */
  Transaction.prototype.commit = function(callback) {
    this.verify();
    var url = crypton.url() + '/transaction/' + this.id + '/commit?sid=' + crypton.sessionId;
    superagent.post(url)
      .withCredentials()
      .end(function(res) {
        if (!res.body || res.body.success !== true) {
          callback(res.body.error);
          return;
        }

        callback();
      });
  };

  /**!
   * ### abort(callback)
   * Ask the server to mark the transaction as aborted
   *
   * Calls back without error if successful
   *
   * Calls back with error if unsuccessful
   *
   * @param {Function} callback
   */
  Transaction.prototype.abort = function(callback) {
    this.verify();
    var url = crypton.url() + '/transaction/' + this.id + '?sid=' + crypton.sessionId;
    superagent.del(url)
      .withCredentials()
      .end(function(res) {
        if (!res.body || res.body.success !== true) {
          callback(res.body.error);
          return;
        }

        callback();
      });
  };

  /**!
   * ### verify()
   * Ensure the transaction is valid to be worked with
   *
   * Throws if unsuccessful
   */
  Transaction.prototype.verify = function() {
    if (!this.id) {
      throw new Error('Invalid transaction');
    }
  };

  /**!
   * ### verifyChunk(chunk)
   * Ensure the provided `chunk` is fit to be sent to server
   *
   * Throws if unsuccessful
   *
   * @param {Object} chunk
   */
  Transaction.prototype.verifyChunk = function(chunk) {
    if (!chunk || !~this.types.indexOf(chunk.type)) {
      throw new Error('Invalid transaction chunk type');
    }
  };

})();

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

'use strict';

var assert = chai.assert;

// most of the transaction functions are simply
// HTTP calls, which are tested in integration tests

describe('Transactions', function() {
  describe('default properties', function() {
    it('should have the correct types array', function() {
      var tx = new crypton.Transaction(null);
      var expectedTypes = [
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

      assert.deepEqual(tx.types, expectedTypes);
    });
  });

  describe('save()', function() {
    // TODO how should we test this outside of integration tests?
  });

  describe('verify()', function() {
    it('should throw if there is no transaction id', function() {
      var err = null;
      var tx = new crypton.Transaction(null);

      try {
        tx.verify();
      } catch (e) {
        err = e;
      }

      assert.equal(err.message, 'Invalid transaction');
    });

    it('should not throw if there is a transaction id', function() {
      var err = null;
      var tx = new crypton.Transaction(null);
      tx.id = 'foo';

      try {
        tx.verify();
      } catch (e) {
        err = e;
      }

      assert.equal(err, null);
    });
  });

  describe('verifyChunk()', function() {
    it('should throw if not given chunk', function() {
      var err = null;
      var tx = new crypton.Transaction(null);

      try {
        tx.verifyChunk();
      } catch (e) {
        err = e;
      }

      assert.equal(err.message, 'Invalid transaction chunk type');
    });

    it('should throw if chunk has invalid type', function() {
      var err = null;
      var tx = new crypton.Transaction(null);

      try {
        tx.verifyChunk({
          type: 'nope',
        });
      } catch (e) {
        err = e;
      }

      assert.equal(err.message, 'Invalid transaction chunk type');
    });

    it('should not throw if chunk has valid type', function() {
      var err = null;
      var tx = new crypton.Transaction(null);

      try {
        tx.verifyChunk({
          type: 'addContainer',
        });
      } catch (e) {
        err = e;
      }

      assert.equal(err, null);
    });
  });
});

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

// These are shallow tests to ensure compatibility
// with crypton if the diff library is switched out.
// We will assume that the diff library is
// well tested including edge cases

describe('Differ', function() {
  it('should return null if unchanged', function() {
    var old = {};
    var current = {};
    var delta = crypton.diff.create(old, current);
    assert.equal(delta, null);
  });

  // TODO what other types of structures should we test here?
  it('should return the appropriate output for changes', function() {
    var old = {};
    var current = {
      foo: { bar: 'baz' },
    };
    var delta = crypton.diff.create(old, current);
    assert.deepEqual(delta, {
      foo: [{ bar: 'baz' }],
    });
  });
});

describe('Patcher', function() {
  it('should return null if given null delta', function() {
    var old = {};
    var delta = null;
    var current = crypton.diff.apply(old, delta);
    assert.equal(current, null);
  });

  // TODO how else should we test this?
  // TODO should we make the abstraction async and wrap patch() in a try/catch?
  it('should err if given non-null non-object delta', function() {
    var old = {};
    var delta = [{
      foo: {
        bar: ['baz'],
      },
    }];

    try {
      crypton.diff.apply(delta, old);
    } catch (e) {
      assert.equal(e.message, 'cannot apply patch at "/": object expected');
    }
  });

  it('should not change anything if given blank input', function() {
    var old = {};
    var delta = {};
    var current = crypton.diff.apply(delta, old);
    assert.deepEqual(old, current);
  });

  // TODO how else should we test this?
  it('should return the appropriate output for changes', function() {
    var old = {};
    var delta = {
      foo: [{
        bar: 'baz',
      }]
    };

    var current = crypton.diff.apply(delta, old);
    assert.deepEqual(current, {
      foo: { bar: 'baz' },
    });
  });
});

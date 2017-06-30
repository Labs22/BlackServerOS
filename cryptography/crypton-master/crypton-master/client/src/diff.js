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

  var Diff = crypton.diff = {};

  /**!
   * ### create(old, current)
   * Generate an object representing the difference between two inputs
   *
   * @param {Object} old
   * @param {Object} current
   * @return {Object} delta
   */
  Diff.create = function(old, current) {
    var delta = jsondiffpatch.diff(old, current);
    return delta;
  };

  // TODO should we switch the order of these arguments?
  /**!
   * ### apply(delta, old)
   * Apply `delta` to `old` object to build `current` object
   *
   * @param {Object} delta
   * @param {Object} old
   * @return {Object} current
   */
  Diff.apply = function(delta, old) {
    var current = JSON.parse(JSON.stringify(old));
    jsondiffpatch.patch(current, delta);
    return current;
  };

})();

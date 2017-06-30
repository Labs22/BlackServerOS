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

  function Errors() {}

  Errors.prototype = {
    // Crypton system error strings
    ARG_MISSING_CALLBACK: 'Callback argument is required',
    ARG_MISSING_STRING: 'String argument is required',
    ARG_MISSING_OBJECT: 'Object argument is required',
    ARG_MISSING: 'Missing required argument',
    PROPERTY_MISSING: 'Missing object property',
    UNWRAP_KEY_ERROR: 'Cannot unwrap session key',
    DECRYPT_CIPHERTEXT_ERROR: 'Cannot decrypt ciphertext',
    UPDATE_PERMISSION_ERROR: 'Update permission denied',
    LOCAL_ITEM_MISSING: 'Cannot delete local Item, not currently cached',
    PEER_MESSAGE_SEND_FAILED: 'Cannot send message to peer',
  };

  crypton.errors = new Errors();

})();

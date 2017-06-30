/* Crypton Server, Copyright 2013 SpiderOak, Inc.
 *
 * This file is part of Crypton Server.
 *
 * Crypton Server is free software: you can redistribute it and/or modify it
 * under the terms of the Affero GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Crypton Server is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Affero GNU General Public
 * License for more details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * along with Crypton Server.  If not, see <http://www.gnu.org/licenses/>.
*/

'use strict';

var fs = require('fs');
var assert = require('assert');

module.exports = {
  readFileSync: function readFileSync(path, encoding, length, position) {
    position = position || 0;
    var descriptor = fs.openSync(path, 'r');
    var contents = new Buffer(length);
    contents.fill(0);
    var bytesRead = fs.readSync(descriptor, contents, 0, length, position);
    assert(bytesRead === length);
    fs.closeSync(descriptor);
    return encoding ? contents.toString(encoding) : contents;
  }
};

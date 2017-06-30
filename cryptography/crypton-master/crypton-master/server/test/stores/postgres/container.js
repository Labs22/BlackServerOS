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

var assert = require('assert');
var container = require('../../../lib/stores/postgres/container');

// TODO can this be executed after the transaction tests?
// this will pass no matter what right now, it should check
// if the records.length > 0
describe('Postgres driver', function () {
  describe('Container', function () {
    describe('getContainerRecords()', function () {
      it('should return container records for known container', function (done) {
        var containerNameHmac = 'exists';
        var accountId = 2;
        container.getContainerRecords(containerNameHmac, accountId, function (err) {
          assert.equal(err, null);
          done();
        });
      });
    });
  });
});

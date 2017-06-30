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
var postgres = require('../../../lib/stores/postgres');

describe('Postgres driver', function () {
  describe('Utility functions', function () {
    describe('camelize()', function () {
      it('should camelize a word', function () {
        var ret = postgres.util.camelize('camel_word');
        assert.equal(ret, 'camelWord');
      });
    });

    describe('camelizeObject()', function () {
      it('should camelize an object', function () {
        var obj = {
          foo_bar: 'baz',
          zerp_glurp: 'nurp'
        };

        var ret = postgres.util.camelizeObject(obj);
        assert.deepEqual(ret, {
          fooBar: 'baz',
          zerpGlurp: 'nurp'
        });
      });
    });

    describe('connect()', function () {
      it('should return a postgres client', function (done) {
        assert.doesNotThrow(function () {
          postgres.connect(function (client) {
            assert.equal(client.user, 'crypton_test_user');
            done();
          });
        });
      });
    });
  });
});

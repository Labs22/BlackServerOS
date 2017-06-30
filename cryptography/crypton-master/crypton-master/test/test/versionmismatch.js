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

describe('Version Mismatch checking', function () {
  this.timeout(50000);

  var REAL_VERSION = new String(crypton.version);

  describe('Account generation', function () {
    it('should refuse registrations when versions mismatch', function (done) {
      if (semver.lte(crypton.version, '0.2.0')) {
        crypton.version = semver.inc(crypton.version, 'minor');
      } else {
        crypton.version = semver.inc(crypton.version, 'major');
      }
      crypton.generateAccount('mismatchname', 'pass', function (err, account) {
	assert.equal(err, null);
	if (err) {
	  done();
	}
        crypton.version = REAL_VERSION;
        assert.equal(err, 'Server and client version mismatch');
        done();
      });
    });
  });

  describe('Authorization', function () {
    it('should refuse authorization when versions mismatch', function (done) {
      crypton.version = '0.27';
      crypton.authorize('mismatchedname2', 'pass', function (err, session) {
        crypton.version = REAL_VERSION;
        assert.equal(err, 'Server and client version mismatch');
        done();
      });
    });
  });
});

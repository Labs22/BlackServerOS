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
   * # Card
   *
   * ````
   * var  = new crypton.Card();
   * ````
   */
  var Card = crypton.Card = function Card() {};

  /**!
   * ### createIdCard(fingerprint, username, appname, domId)
   *
   * returns canvas element
   *
   * @param {String} fingerprint
   * @param {String} username
   * @param {String} appname
   * @param {String} domId [optional]
   */
  Card.prototype.createIdCard =
    function(fingerprint, username, appname, domId) {
      if (!domId) {
        domId = 'id-card';
      }

      var fingerArr = this.createFingerprintArr(fingerprint);
      var canvas = document.createElement('canvas');
      canvas.width = 290;
      canvas.height = 480;
      canvas.setAttribute('id', domId);

      var ctx = canvas.getContext('2d');
      var x = 20;
      var y = 15;

      ctx.fillStyle = 'white';
      ctx.fillRect(0, 0, 290, 480);

      ctx.fillStyle = '#CCCCCC';
      ctx.fillRect(0, 0, 289, 1);
      ctx.fillRect(0, 0, 1, 479);
      ctx.fillRect(289, 0, 1, 479);
      ctx.fillRect(0, 479, 290, 1);

      ctx.fillStyle = 'black';
      y = y + 10;
      ctx.font = '11px sans-serif';
      ctx.fillText(appname, x, y);
      y = y + 12;
      var qrCodeCanvas = this.createQRCode(fingerArr, username, appname);
      ctx.drawImage(qrCodeCanvas, 23, y);
      y = y + 265;
      ctx.fillText(username, x, y);

      return canvas;
    };

  /**!
   * ### createQRCode(fingerprint, username, appname)
   *
   * returns canvas element
   *
   * @param {Array} fingerArr
   * @param {String} username
   * @param {String} appname
   */
  Card.prototype.createQRCode = function(fingerArr, username, appname) {

    var qrValue = JSON.stringify({
      f: fingerArr.join(' '),
      u: username,
      a: appname,
    });

    var qrCodeCanvas = qr.canvas({
      value: qrValue,
      level: 'H',
      size: 10, // 250 X 250 PX
    });

    return qrCodeCanvas;
  };

  /**!
   * ### createIdentigrid(fingerprint, username, appname)
   *
   * returns canvas element
   *
   * @param {Array} colorArr
   */
  Card.prototype.createIdentigrid = function(colorArr) {
    var canvas = document.createElement('canvas');
    canvas.width = 200;
    canvas.height = 200;
    var ctx = canvas.getContext('2d');
    var x = 0;
    var y = 0;
    var w = 50;
    var h = 50;

    for (var idx in colorArr) {
      ctx.fillStyle = colorArr[idx];
      ctx.fillRect(x, y, w, h);
      x = x + 50;
      if (x === 200) {
        x = 0;
        y = y + 50;
      }
    }

    return canvas;
  };

  /**!
   * ### createColorArr(fingerprint)
   *
   * returns Array
   *
   * @param {String} fingerArr
   */
  Card.prototype.createColorArr = function(fingerArr) {
    // pad the value out to 6 digits:
    var count = 0;
    var paddingData = fingerArr.join('');
    var colorArr = [];
    var REQUIRED_LENGTH = 6;
    for (var idx in fingerArr) {
      var pad = REQUIRED_LENGTH - fingerArr[idx].length;
      if (pad === 0) {
        colorArr.push('#' + fingerArr[idx]);
      } else {
        var color = '#' + fingerArr[idx];
        for (var i = 0; i < pad; i++) {
          color = color + paddingData[count];
          count++;
        }

        colorArr.push(color);
      }
    }

    return colorArr;
  };

  /**!
   * ### createFingerprintArr(fingerprint)
   *
   * returns Array
   *
   * @param {String} fingerprint
   */
  Card.prototype.createFingerprintArr = function(fingerprint) {
    if (fingerprint.length !== 64) {
      var err = 'Fingerprint has incorrect length';
      console.error(err);
      throw new Error(err);
    }

    fingerprint = fingerprint.toUpperCase();
    var fingerArr = [];
    var i = 0;
    var segment = '';
    for (var chr in fingerprint) {
      segment = segment + fingerprint[chr];
      i++;
      if (i === 4) {
        fingerArr.push(segment);
        i = 0;
        segment = '';
        continue;
      }
    }

    return fingerArr;
  };

  /**!
   * ### generateQRCodeInput(fingerprint, username, application)
   *
   * returns Array
   *
   * @param {String} fingerprint
   */
  Card.prototype.generateQRCodeInput = function(fingerprint, username, application, url) {
    if (!url) {
      url = '';
    }

    var json = JSON.stringify({
      fingerprint: fingerprint,
      username: username,
      application: application,
    });
    return json;
  };

}());

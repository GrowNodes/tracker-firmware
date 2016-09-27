function submitWIFIInfo() {
  var wifiData = {};
  wifiData.network = document.getElementById('wifi_name').value;
  wifiData.password = document.getElementById('wifi_password').value;

  nanoajax.ajax({url: '/wifi', method: 'POST', headers:{'Content-Type': 'application/json'}, body: wifiData}, function (code, responseText, request) {
    // code is response code
    // responseText is response body as a string
    // request is the xmlhttprequest, which has `getResponseHeader(header)` function
  });
}

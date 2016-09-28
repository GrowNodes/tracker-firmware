var WIFI_STATUS_CONNECTED = 'CONNECTED';
function submitWIFIInfo() {
  var wifiData = {};
  wifiData.ssid = document.getElementById('wifi_name').value;
  wifiData.password = document.getElementById('wifi_password').value;

  showInfoMessage('Iniciando conexão à rede "' + wifiData.ssid + '". Aguarde alguns instantes...', 'Conectando o dispositivo ao Wi-Fi');
  nanoajax.ajax({url: '/wifi-connect', method: 'POST', headers:{'Content-Type': 'application/json'}, body: wifiData}, function (code, responseText, request) {
    var responseJSON = JSON.parse(responseText);
    if (code === 202) {
      showInfoMessage('Rede identificada. Finalizando conexão...', 'Conectando o dispositivo ao Wi-Fi');
      checkConnectionEstabilished();

    }else if (code===400) {
      showErrorMessage('Houve um erro ao conectar o dispositivo à rede Wi-Fi. Detalhes: ' + responseJSON.error, 'Erro ao conectar-se a rede Wi-Fi', 2);

    }else{
      showErrorMessage('Houve um erro ao conectar o dispositivo à rede Wi-Fi. Detalhes: ' + responseJSON.error, 'Erro ao conectar-se a rede Wi-Fi', 2);
    }
  });
}

var wifiConnectionCheckAttempts = 0;
function checkConnectionEstabilished() {
  //request for /wifi-status to check whether the connection successfully estabilishes
  nanoajax.ajax({url: '/wifi-status', method: 'GET', headers:{'Content-Type': 'application/json'}}, function (code, responseText, request) {
    if (code === 200 && responseText.indexOf('status')!==-1) {
      var status = JSON.parse(responseText).status;
      if (status===WIFI_STATUS_CONNECTED) {
        showSuccessMessage('Conexão estabelecida com sucesso!', 'Dispositivo conectado à rede Wi-fi');
        //iniciar procedimento de login no stutzthings
        return;

      }else if (wifiConnectionCheckAttempts > 30) {
        showErrorMessage('Não foi possível conectar o dispositivo à rede Wi-Fi. Detalhes: Número de tentativas excedido (timeout).', 'Erro ao conectar-se a rede Wi-Fi');
        return;

      }else{
        wifiConnectionCheckAttempts++;
        setTimeout(checkConnectionEstabilished, 1000);
        return;
      }
    }else{
      showErrorMessage('Houve um erro ao conectar o dispositivo à rede Wi-Fi. Detalhes: ' + responseJSON.error, 'Erro ao conectar-se a rede Wi-Fi');
      return;
    }

  });

}

/**
 *  @msgType : 1-Succes, 2-Error (defaults to 1)
 **/
function showStatusMessage(msg, title, msgType) {
  if (typeof(msgType) === "undefined" || !msgType) {
    msgType = 1;
  }
  document.getElementById('statusMessages').style.display = 'block';
  document.getElementById('statusMessages').innerHTML = msg;
}

function hideStatusMessage() {
  document.getElementById('statusMessages').style.display = 'none';
}

function showInfoMessage(msg, title) {
  showStatusMessage(msg, title, 0);
}
function showSuccessMessage(msg, title) {
  showStatusMessage(msg, title, 1);
}
function showErrorMessage(msg, title) {
  showStatusMessage(msg, title, 2);
}

// in nanoajax
// code is response code
// responseText is response body as a string
// request is the xmlhttprequest, which has `getResponseHeader(header)` function

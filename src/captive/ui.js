// Steps to setup
// 1- user connects to device's wifi AP, `Homie-xxxxxxxx`
// 2- device shows config page
// 3- the page list to use the available wifi networks to connect. (using the `/networks` Homie API endpoint)
// 4- user selects one of the enlisted networks to connect and input the network password and connects using the `/wifi/connect` Homie API endpoint.
// 4.1- Using the `/wifi/status` Homie API endpoint, check the connection status and present a feedback to the user
// 5- captive enables the device as a internet proxy using the `/proxy/control` Home API endpoint
// 6- user inputs user and pass for stutzthings.com
// 7- page makes a POST with user/pass to the `` stutzthings API endpoint and receives an deviceID, used to register the mqtt topic
// 8- page makes a POST to the `/config` Homie API endpoint to store the configuration
// 9- the device will restart and will be ready to go!

var wifiConnectionCheckAttempts = 0;

var WIFI_STATUS_CONNECTED = 'CONNECTED';

//Dynamic DOM elements
var ui_wifi_password_input_group = document.createElement('div');
ui_wifi_password_input_group.innerHTML = '<input id="wifi_password" type="password" name="wifi_password" placeholder="digite a senha do Wi-Fi"></input>';

var ui_wifi_connect_button_group = document.createElement('div');
ui_wifi_connect_button_group.innerHTML = '<input type="button" name="wifi_connect_action" value="Conectar" onclick="connectToSelectedWiFi()">';


function showAvailableNetworks() {
  showInfoMessage('Buscando por redes Wi-Fi para iniciar o processo de configuração do dispositivo na Internet...', 'Procurando redes Wi-Fi disponíveis');
  nanoajax.ajax({url: '/networks', method: 'GET', headers:{'Content-Type': 'application/json'}}, function (code, responseText, request) {
    var responseJSON = JSON.parse(responseText);
    if (code === 200) {
      hideStatusMessage();

      // show wi-fi networks list
      var wifiListElement = el('wifi_list');
      wifiListElement.appendChild(ui_createWiFiNetworkDOMElement('li'));

    }else if (code===503) {
      showErrorMessage('Erro ao listar redes Wi-Fi disponíveis. Detalhes: ' + responseJSON.error, 'Erro ao listar redes Wi-Fi disponiveis');

    }else{
      showErrorMessage('Houve um erro inesperado ao conectar listar as redes Wi-Fi disponíveis. Detalhes: ' + responseJSON.error, 'Erro ao listar redes Wi-Fi disponiveis');
    }
  });
}

function ui_selectWiFiNetwork(ssid) {
  el('wifi_name').value = ssid;
  
  //reset the style of the current selected element and set the style in the new element
  //open the password box right below the selected network
}
function ui_createWiFiNetworkDOMElement(ssid, rssi, encryption) {
  var li = document.createElement('li');
  li.setAttribute('data-network-name', ssid);
  li.innerHTML = '<em>' + ssid + '</em>' + '<button onclick="ui_selectWiFiNetwork(\'' + ssid + '\')">selecionar</button>';
  li.onclick = function() {
    ui_selectWiFiNetwork(ssid);
  };
  return el;
}

function connectToSelectedWiFi() {
  var wifiData = {};
  wifiData.ssid = el('wifi_name').value;
  wifiData.password = el('wifi_password').value;

  showInfoMessage('Iniciando conexão à rede "' + wifiData.ssid + '". Aguarde alguns instantes...', 'Conectando o dispositivo ao Wi-Fi');
  nanoajax.ajax({url: '/wifi/connect', method: 'POST', headers:{'Content-Type': 'application/json'}, body: wifiData}, function (code, responseText, request) {
    var responseJSON = JSON.parse(responseText);
    if (code === 202) {
      showInfoMessage('Rede identificada. Finalizando conexão...', 'Conectando o dispositivo ao Wi-Fi');
      checkConnectionEstabilished();

    }else if (code===400) {
      showErrorMessage('Houve um erro ao conectar o dispositivo à rede Wi-Fi. Detalhes: ' + responseJSON.error, 'Erro ao conectar-se a rede Wi-Fi');

    }else{
      showErrorMessage('Houve um erro ao conectar o dispositivo à rede Wi-Fi. Detalhes: ' + responseJSON.error, 'Erro ao conectar-se a rede Wi-Fi');
    }
  });
}

function checkConnectionEstabilished() {
  //request for /wifi/status to check whether the connection was successfully estabilished
  nanoajax.ajax({url: '/wifi/status', method: 'GET', headers:{'Content-Type': 'application/json'}}, function (code, responseText, request) {
    if (code === 200 && responseText.indexOf('status')!==-1) {
      var status = JSON.parse(responseText).status;
      if (status===WIFI_STATUS_CONNECTED) {
        showSuccessMessage('Conexão estabelecida com sucesso!', 'Dispositivo conectado à rede Wi-fi');
        //iniciar procedimento de login no stutzthings
        hideElement('step_wifi_connect');
        showElement('step_device_enroll');

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
  * Invoke stutzthings API endpoint that enrolls the device to the stutzthings user's account
  **/
function enrollDeviceToStutzThingsAccount() {
  //request for /proxy/control to enable internet access using the device as AP
  nanoajax.ajax({url: '/proxy/control', method: 'PUT', headers:{'Content-Type': 'application/json'}, body: { "enable": true } }, function (code, responseText, request) {
    if (code === 200) {

      var account_id = el('stutzthings_account_id').value;
      var device_id = 'tracker';
      var registerPayload =  {
        account_password  : el('stutzthings_password').value,
        custom_name       : el('stutzthings_custom_name').value,
        hardware_id       : el('stutzthings_hardware_id').value
      };

      nanoajax.ajax({url: 'http://api.stutzthings.com/v1/' + account_id + '/' + device_id, method: 'POST', headers:{'Content-Type': 'application/json'}, body: registerPayload }, function (code, responseText, request) {
        var responseJSON = JSON.parse(responseText);
        if (code === 200) {
          showSuccessMessage('Seu dispositivo foi registrado com sucesso em sua conta StutzThings e está pronto para ser usado. Ele reinciará agora para carregar a nova configuração e já poderá ser usado.','Configuração concluída com sucesso!')

          hideElement('step_device_enroll');
          showElement('step_final');

        }else{
          showErrorMessage('Houve um erro inesperado ao registrar o dispositivo em sua conta. Detalhes: ', 'Erro ao registrar dispositivo na sua conta StutzThings')
        }
      });

    }else{
      showErrorMessage('Houve um erro ao habilitar a conexão com internet por meio do dispositivo. Detalhes: ' + responseJSON.error, 'Erro ao tantar habilitar a função proxy do dispositivo');
      return;
    }

  });
}




function el(id){
  return document.getElementById(id);
}
function showElement(id) {
  el(id).style.display = 'block';
}
function hideElement(id) {
  el(id).style.display = 'none';
}
/**
 *  @msgType : 1-Succes, 2-Error (defaults to 1)
 **/
function showStatusMessage(msg, title, msgType) {
  if (typeof(msgType) === "undefined" || !msgType) {
    msgType = 1;
  }
  el('statusMessages').style.display = 'block';
  el('statusMessages').innerHTML = msg;
}

function hideStatusMessage() {
  el('statusMessages').style.display = 'none';
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


//start the configuration process
showElement('step_wifi_connect');

// in nanoajax
// code is response code
// responseText is response body as a string
// request is the xmlhttprequest, which has `getResponseHeader(header)` function

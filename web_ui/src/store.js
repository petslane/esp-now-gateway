import Vue from 'vue';
import Vuex from 'vuex';
import api from "./api";

Vue.use(Vuex);

export default new Vuex.Store({
  strict: process.env.NODE_ENV !== 'production',
  state: {
    webSocketConnected: false,
    webSocketMessages: [],
    apName: undefined,
    currentWifiNetwork: '',
    currentWifiConnected: false,
    currentWifiError: '',
    wifiNetworks: [],
    scanningWifiNetworks: false,
    nowDevices: [],
      stats: {
          buffer1_free: 0,
          buffer1_size: 0,
          buffer2_free: 0,
          buffer2_size: 0,
          missed_incoming_messages: 0,
          msgs_failed_sent: 0,
          msgs_received: 0,
          msgs_success_sent: 0,
      },
  },
  getters: {
      getWebSocketMessages: (state) => {
          return state.webSocketMessages;
      },
      getWebSocketConnected: (state) => state.webSocketConnected,
      getNowDevices: (state) => state.nowDevices,
  },
  mutations: {
      webSocketStatus (state, payload) {
          state.webSocketConnected = payload;
          state.webSocketMessages.push(['status', new Date(), payload]);
      },
      webSocketError (state, payload) {
          state.webSocketMessages.push(['error', new Date(), payload]);
      },
      webSocketData (state, payload) {
          try {
              const data = JSON.parse(payload.data);
              if (!data) {
                  state.webSocketMessages.push(['data', new Date(), payload]);
                  if (state.webSocketMessages.length > 100) {
                      state.webSocketMessages.splice(0, state.webSocketMessages.length - 100);
                  }
                  return;
              }
              if (data.type === 'stats') {
                  const properties = [
                      'buffer1_free',
                      'buffer1_size',
                      'buffer2_free',
                      'buffer2_size',
                      'missed_incoming_messages',
                      'msgs_failed_sent',
                      'msgs_received',
                      'msgs_success_sent',
                  ];
                  properties.map((p) => {
                      Vue.set(state.stats, p, data[p]);
                  });
              } else {
                  state.webSocketMessages.push(['data', new Date(), payload]);
                  if (state.webSocketMessages.length > 100) {
                      state.webSocketMessages.splice(0, state.webSocketMessages.length - 100);
                  }
              }
          } catch (e) {
              state.webSocketMessages.push(['data', new Date(), payload]);
              if (state.webSocketMessages.length > 100) {
                  state.webSocketMessages.splice(0, state.webSocketMessages.length - 100);
              }
          }
      },
      clearLogMessages (state) {
          Vue.set(state, 'webSocketMessages', []);
      },
      setNowDevices (state, payload) {
          Vue.set(state, 'nowDevices', payload);
      },
      setWifiNetworks(state, payload) {
          payload.sort((a, b) => {
              return a[2] < b[2];
          });
          Vue.set(state, 'wifiNetworks', payload);
      },
      setCurrentWifiNetwork(state, payload) {
          Vue.set(state, 'currentWifiNetwork', payload.current);
          Vue.set(state, 'currentWifiConnected', payload.current_connected);
          Vue.set(state, 'currentWifiError', payload.current_error);
      },
      setScanningWifiNetworks(state, payload) {
          Vue.set(state, 'scanningWifiNetworks', payload);
      },
      setWifiSettings(state, payload) {
        Vue.set(state, 'apName', payload.ap_name);
        Vue.set(state, 'currentWifiNetwork', payload.current);
        Vue.set(state, 'currentWifiConnected', payload.current_connected);
        Vue.set(state, 'currentWifiError', payload.current_error);
    },
  },
  actions: {
      webSocketStatus ({ commit }, payload) {
          commit('webSocketStatus', payload);
      },
      webSocketError ({ commit }, payload) {
          commit('webSocketError', payload);
      },
      webSocketData ({ commit }, payload) {
          commit('webSocketData', payload);
      },
      clearLogMessages ({ commit }) {
          commit('clearLogMessages');
      },
      fetchNowDevices ({ commit }) {
          return api('get_devices')
              .then(json => {
                  commit('setNowDevices', json.data);
              });
      },
      deleteDevice ({ commit }, id) {
          return api('delete_device', { id })
              .then(json => {
                  commit('setNowDevices', json.data);
              });
      },
      saveDevice ({ commit }, payload) {
          return api('save_device', {
              id: payload.id,
              name: payload.name,
              mac: payload.mac,
          })
              .then(json => {
                  commit('setNowDevices', json.data);
              });
      },
      async getWifiNetworks ({ commit, dispatch }) {
          const json = await api('get_wifi_networks');

          if (json.scanning) {
            setTimeout(() => dispatch('getWifiNetworks'), 1000);
            return;
          }

          commit('setWifiNetworks', json.data);
          commit('setCurrentWifiNetwork', json);
          commit('setScanningWifiNetworks', false);
      },
      async scanWifiNetworks ({ commit, dispatch }) {
          commit('setScanningWifiNetworks', true);
          await api('scan_wifi_networks');
          setTimeout(() => dispatch('getWifiNetworks'), 1000);
      },
      async getWifiSettings ({ commit, dispatch }) {
          const result = await api('get_wifi_settings');

          commit('setWifiSettings', result);
      },
      async connectWifi ({ commit }, payload) {
          await api('connect_wifi', {
              ssid: payload.ssid,
              pass: payload.pass,
          });

          const wait = (n) => new Promise((resolve, reject) => setTimeout(resolve, n));
          const get = () => new Promise((resolve, reject) => {
              api('get_wifi_networks')
                  .then((data) => resolve(data))
                  .catch((e) => reject(e));
          });
          
          let status = null;
          await wait(2000);
          for (let i = 0; i < 20; i++) {
              try {
                  const json = await get();
                  status = json.current === payload.ssid;
                  break;
              } catch (e) {
                  await wait(2000);
              }
          }

          return status;
      },
      async saveAP ({ commit, dispatch }, apName) {
          const result = await api('save_ap_name', { name: apName });

          commit('setWifiSettings', result);
      },
  }
});

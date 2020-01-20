import Vue from 'vue';
import Vuex from 'vuex';
import api from "./api";

Vue.use(Vuex);

export default new Vuex.Store({
  strict: process.env.NODE_ENV !== 'production',
  state: {
    webSocketConnected: false,
    webSocketMessages: [],
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
  }
});

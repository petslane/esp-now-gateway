import Vue from 'vue';
import api from "../api";

export default {
    namespaced: true,
    strict: process.env.NODE_ENV !== 'production',
    state: {
        apName: undefined,
        currentWifiNetwork: '',
        currentWifiConnected: false,
        currentWifiError: '',
        wifiNetworks: [],
        scanningWifiNetworks: false,
        nowMac: '',
        authUsername: '',
    },
    getters: {},
    mutations: {
        setWifiNetworks(state, payload) {
            Vue.set(state, 'wifiNetworks', payload.filter(n => n && n.length));
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
            Vue.set(state, 'nowMac', payload.now_mac);
        },
        setNowMac(state, payload) {
            Vue.set(state, 'nowMac', payload.now_mac);
        },
        setAuthUsername(state, payload) {
            Vue.set(state, 'authUsername', payload);
        },
    },
    actions: {
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
        async saveNowMac ({ commit, dispatch }, mac) {
            const result = await api('save_now_mac', { mac });

            commit('setNowMac', result);
        },
        async setAuthUsername({ commit, dispatch }, { username, password }) {
            const result = await api('set_auth_username', { username, password });

            commit('setAuthUsername', result.username);
        },
        async getAuthUsername({ commit, dispatch }, mac) {
            const result = await api('get_auth_username', {});

            commit('setAuthUsername', result.username);
        },
    },
};

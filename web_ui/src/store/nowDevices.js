import Vue from 'vue';
import api from "../api";

export default {
    namespaced: true,
    strict: process.env.NODE_ENV !== 'production',
    state: {
        devices: [],
    },
    getters: {
        getNowDevices: (state) => state.devices,
    },
    mutations: {
        setNowDevices(state, payload) {
            Vue.set(state, 'devices', payload);
        },
    },
    actions: {
        fetch({ commit }) {
            return api('get_devices')
                .then(json => {
                    commit('setNowDevices', json.data);
                });
        },
        remove({ commit }, id) {
            return api('delete_device', { id })
                .then(json => {
                    commit('setNowDevices', json.data);
                });
        },
        save({ commit }, payload) {
            return api('save_device', {
                id: payload.id,
                name: payload.name,
                mac: payload.mac,
            })
                .then(json => {
                    commit('setNowDevices', json.data);
                });
        },
    },
};

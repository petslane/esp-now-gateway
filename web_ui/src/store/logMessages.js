import Vue from 'vue';

export default {
    namespaced: true,
    strict: process.env.NODE_ENV !== 'production',
    state: {
        webSocketConnected: false,
        webSocketMessages: [],
    },
    getters: {
        getMessages: (state) => {
            return state.webSocketMessages;
        },
        getConnected: (state) => state.webSocketConnected,
    },
    mutations: {
        clearLogMessages (state) {
            Vue.set(state, 'webSocketMessages', []);
        },
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
                    this.commit('stats/setStatsData', data);
                } else if (data.type === 'now_received') {
                    state.webSocketMessages.push(['in', new Date(), {
                        from: data.from,
                        msg: data.msg,
                    }]);
                } else if (data.type === 'sending') {
                    state.webSocketMessages.push(['out', new Date(), {
                        id: data.id,
                        to: data.to,
                        msg: data.msg,
                        delivered: undefined,
                    }]);
                } else if (data.type === 'sent' || data.type === 'not_sent') {
                    const i = state.webSocketMessages.findIndex((log) => log[2].id === data.id && log[2].delivered === undefined);
                    if (i >= 0) {
                        Vue.set(state.webSocketMessages[i][2], 'delivered', data.type === 'sent' ? new Date() : false);
                    }
                } else {
                    state.webSocketMessages.push(['data', new Date(), payload]);
                    if (state.webSocketMessages.length > 100) {
                        state.webSocketMessages.splice(0, state.webSocketMessages.length - 100);
                    }
                }
            } catch (e) {
                debugger;
                state.webSocketMessages.push(['data', new Date(), payload]);
                if (state.webSocketMessages.length > 100) {
                    state.webSocketMessages.splice(0, state.webSocketMessages.length - 100);
                }
            }
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
    },
};

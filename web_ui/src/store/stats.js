import Vue from 'vue';

export default {
    namespaced: true,
    strict: process.env.NODE_ENV !== 'production',
    state: {
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
    getters: {},
    mutations: {
        setStatsData (state, data) {
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
        },
    },
    actions: {},
};

import Vue from 'vue';
import Vuex from 'vuex';
import config from "./config";
import nowDevices from "./nowDevices";
import logMessages from "./logMessages";
import stats from "./stats";

Vue.use(Vuex);

export default new Vuex.Store({
  strict: process.env.NODE_ENV !== 'production',
  modules: {
    config: config,
    nowDevices: nowDevices,
    logMessages: logMessages,
    stats: stats,
  },
});

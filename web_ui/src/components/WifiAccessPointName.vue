<template>
    <div>
        <h4>Access Point</h4>
        <small><i>AP will be enabled only on missing WiFi connection</i></small>
        <div class="content">
            <ui-input placeholder="AP name" class="content" v-model="apNameInput" />
            <span class="spacer"></span>
            <ui-button @click="apSave()">Save</ui-button>
        </div>
    </div>
</template>

<script>
    import {mapState} from "vuex";

    export default {
        name: 'WifiAccessPointName',
        data() {
            return {
                apNameInput: '',
            };
        },
        computed: {
            ...mapState('config', {
                apName: 'apName',
            }),
        },
        watch: {
            apName(value) {
                this.apNameInput = value;
            },
        },
        methods: {
            apSave() {
                this.$store.dispatch('config/saveAP', this.apNameInput);
            },
        },
    };
</script>

<style scoped lang="scss"></style>

<template>
    <div>
        <h4>NOW gateway</h4>
        <div class="content">
            <mac-input placeholder="MAC address" class="content" v-model="nowMacInput" />
            <span class="spacer"></span>
            <ui-button @click="nowMacSave()">Save</ui-button>
        </div>
    </div>
</template>

<script>
    import {mapState} from "vuex";

    export default {
        name: 'NowGatewayMac',
        data() {
            return {
                nowMacInput: '',
            };
        },
        computed: {
            ...mapState('config', {
                nowMac: 'nowMac',
            }),
        },
        watch: {
            nowMac(value) {
                this.nowMacInput = value;
            },
        },
        methods: {
            nowMacSave() {
                this.$store.dispatch('config/saveNowMac', this.nowMacInput);
            },
        },
    };
</script>

<style scoped lang="scss"></style>

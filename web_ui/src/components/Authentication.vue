<template>
    <div>
        <h4>Authentication</h4>
        <div class="content">
            <label>
            <input type="checkbox" v-model="useAuth" />
            Require authentication to use Web UI
            </label>
        </div>
        <div class="content">
            <ui-input placeholder="Username" v-model="username" style="flex: 1 0 100px;" v-show="useAuth" />
            <span class="spacer"></span>
            <ui-button @click="showModal = true" v-if="useAuth" :disabled="!username.length">Set password</ui-button>
            <ui-button @click="save()" v-if="!useAuth">Save</ui-button>
        </div>

        <modal
            title="Set password"
            :show="showModal"
            @close="showModal = false"
        >
            <div class="rows">
                <div>Set password for accessing Web UI</div>
                <div class="spacer">&nbsp;</div>
                <div class="content">
                    <ui-input v-model="password" type="password" placeholder="Password" style="flex: 1 0 100px;"></ui-input>
                </div>
                <div class="content">
                    <ui-input v-model="password2" type="password" placeholder="Repeat password" style="flex: 1 0 100px;"></ui-input>
                </div>
                <div class="content">
                    <div class="spacer">&nbsp;</div>
                </div>
                <div class="content">
                    <span style="color: darkred" v-if="!password.length">
                        Password missing
                    </span>
                    <span style="color: darkred" v-if="password.length && password !== password2">
                        Password mismatch
                    </span>
                    <div class="spacer">&nbsp;</div>
                    <ui-button
                        :disabled="!password.length || password !== password2"
                        @click="save().then(() => showModal = false)"
                        class="right"
                        color="blue"
                    >
                        Save
                    </ui-button>
                </div>
            </div>
        </modal>
    </div>
</template>

<script>
    import {mapState} from "vuex";

    export default {
        name: 'Authentication',
        data() {
            return {
                username: '',
                password: '',
                password2: '',
                useAuth: false,
                showModal: false,
            };
        },
        mounted() {
            this.$store.dispatch('config/getAuthUsername');
        },
        computed: {
            ...mapState('config', {
                authUsername: 'authUsername',
            }),
        },
        watch: {
            authUsername(value) {
                this.username = value;
                this.useAuth = !!value.length
            },
        },
        methods: {
            save() {
                return this.$store.dispatch('config/setAuthUsername', {
                    username: this.useAuth && this.username || "",
                    password: this.useAuth && this.username ? this.password : undefined,
                });
            },
        },
    };
</script>

<style scoped lang="scss"></style>

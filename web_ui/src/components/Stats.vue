<template>
    <div>
        <div class="text">
            Incoming buffer
            <ui-progressbar :percent="`${getPercent(stats.buffer1_size, stats.buffer1_free)}%`"></ui-progressbar>
            <b>{{ stats.buffer1_size - stats.buffer1_free }}</b> / <b>{{ stats.buffer1_size }}</b>
            <div class="helper-text">
                Buffer for incoming messages. This is first fast buffer where incoming messages as initially stored.
                Needed to be able process rapidly incoming messages.
            </div>
        </div>
        <div class="text">
            Message buffer
            <ui-progressbar :percent="`${getPercent(stats.buffer2_size, stats.buffer2_free)}%`"></ui-progressbar>
            <b>{{ Math.round((stats.buffer2_size - stats.buffer2_free) / 1000) }}kb</b> / <b>{{ Math.round(stats.buffer2_size / 1000) }}kb</b>
            <div class="helper-text">
                Buffer for incoming and outgoing messages.
            </div>
        </div>
        <div class="text">
            Messages received: <b>{{ stats.msgs_received }}</b>
            <div class="helper-text">
                Messages received by gateway.
            </div>
        </div>
        <div class="text">
            Missed incoming messages: <b>{{ stats.missed_incoming_messages }}</b>
            <div class="helper-text">
                Messages not accepted by gateway because of full <i>incoming buffer</i>.
            </div>
        </div>
        <div class="text">
            Messages sent successfully: <b>{{ stats.msgs_success_sent }}</b>
            <div class="helper-text">
                Messages sent by gateway with successful delivery.
            </div>
        </div>
        <div class="text">
            Messages not delivered: <b>{{ stats.msgs_failed_sent }}</b>
            <div class="helper-text">
                Messages sent by gateway with failed delivery.
            </div>
        </div>
    </div>
</template>

<script>
    import {mapState} from "vuex";

    export default {
        name: 'Stats',
        data() {
            return {

            };
        },
        computed: {
            ...mapState({
                stats: 'stats',
            }),
        },
        methods: {
            getPercent(max, value) {
                return ((max - value) / max) * 100;
            }
        },
    };
</script>

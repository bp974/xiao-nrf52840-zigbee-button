import * as m from 'zigbee-herdsman-converters/lib/modernExtend';
import * as exposes from 'zigbee-herdsman-converters/lib/exposes';
import * as reporting from 'zigbee-herdsman-converters/lib/reporting';

const e = exposes.presets;

const buttonActions = {
    exposes: [
        e.action(['single', 'double', 'hold', 'release']),
        e.numeric('voltage', exposes.access.STATE)
            .withUnit('mV')
            .withDescription('Voltage of the battery in millivolts'),
    ],

    fromZigbee: [
        // Gesture commands sent by the nRF52840 firmware.
        {
            cluster: 'genOnOff',
            type: ['commandToggle', 'commandOn'],
            convert: (model, msg) => {
                if (msg.type === 'commandToggle') {
                    return {action: 'single'};
                }

                if (msg.type === 'commandOn') {
                    return {action: 'double'};
                }
            },
        },
        {
            cluster: 'genLevelCtrl',
            type: ['commandMove', 'commandStop'],
            convert: (model, msg) => {
                if (msg.type === 'commandMove') {
                    return {action: 'hold'};
                }

                if (msg.type === 'commandStop') {
                    return {action: 'release'};
                }

            },
        },
        {
            // Exact nRF battery voltage in millivolts from Basic 0xFF01.
            cluster: 'genBasic',
            type: ['attributeReport', 'readResponse'],
            convert: (model, msg) => {
                const exactVoltage = msg.data['65281'] ?? msg.data[65281];

                if (typeof exactVoltage === 'number' && exactVoltage > 0) {
                    return {voltage: exactVoltage};
                }
            },
        },
    ],

    configure: [
        async (device, coordinatorEndpoint) => {
            const endpoint = device.getEndpoint(10);
            const reportingConfiguration = [{
                // Unknown attributes must include their ZCL data type so
                // zigbee-herdsman can build the configure-reporting request.
                attribute: {ID: 0xFF01, type: 0x21}, // uint16
                minimumReportInterval: 0,
                maximumReportInterval: 21600,
                reportableChange: 10,
            }];

            await reporting.bind(endpoint, coordinatorEndpoint, ['genBasic']);
            await endpoint.configureReporting('genBasic', reportingConfiguration);
            await endpoint.read('genBasic', [0xFF01]);
        },
    ],

    isModernExtend: true,
};

export default {
    fingerprint: [
        {
            modelID: 'XIAO-Zigbee-Button',
            manufacturerName: 'Broskie Applications',
        },
    ],
    model: 'XIAO-Zigbee-Button',
    vendor: 'Broskie Applications',
    description: 'Zigbee single button',
    extend: [
        buttonActions,
        m.battery(),
    ],
};

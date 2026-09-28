import * as m from 'zigbee-herdsman-converters/lib/modernExtend';
import * as exposes from 'zigbee-herdsman-converters/lib/exposes';
import * as reporting from 'zigbee-herdsman-converters/lib/reporting';

const e = exposes.presets;
const buttonNames = ['button_1', 'button_2', 'button_3'];
const gestureNames = ['single', 'double', 'hold', 'release'];
const actionNames = buttonNames.flatMap((button) =>
    gestureNames.map((gesture) => `${button}_${gesture}`));

const buttonActions = {
    exposes: [
        e.action(actionNames),
        e.numeric('voltage', exposes.access.STATE)
            .withUnit('mV')
            .withDescription('Voltage of the battery in millivolts')
            .withCategory('diagnostic'),
    ],

    fromZigbee: [
        // Gesture commands sent by the nRF52840 firmware.
        {
            cluster: 'genOnOff',
            type: ['commandToggle', 'commandOn'],
            convert: (model, msg) => {
                const button = `button_${msg.endpoint.ID - 9}`;

                if (msg.type === 'commandToggle') {
                    return {action: `${button}_single`};
                }

                if (msg.type === 'commandOn') {
                    return {action: `${button}_double`};
                }
            },
        },
        {
            cluster: 'genLevelCtrl',
            type: ['commandMove', 'commandStop'],
            convert: (model, msg) => {
                const button = `button_${msg.endpoint.ID - 9}`;

                if (msg.type === 'commandMove') {
                    return {action: `${button}_hold`};
                }

                if (msg.type === 'commandStop') {
                    return {action: `${button}_release`};
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
    description: 'Zigbee three-button remote',
    extend: [
        buttonActions,
        m.battery(),
    ],
};

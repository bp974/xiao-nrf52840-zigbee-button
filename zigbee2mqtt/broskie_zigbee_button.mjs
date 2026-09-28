import * as m from 'zigbee-herdsman-converters/lib/modernExtend';
import * as exposes from 'zigbee-herdsman-converters/lib/exposes';
import * as reporting from 'zigbee-herdsman-converters/lib/reporting';

const e = exposes.presets;
const gestures = ['single', 'double', 'hold', 'release'];

const gestureConverters = (threeButton) => [
    {
        cluster: 'genOnOff',
        type: ['commandToggle', 'commandOn'],
        convert: (model, msg) => {
            const gesture = msg.type === 'commandToggle' ? 'single' : 'double';

            if (!threeButton) {
                return msg.endpoint.ID === 10 ? {action: gesture} : undefined;
            }

            return {action: `button_${msg.endpoint.ID - 9}_${gesture}`};
        },
    },
    {
        cluster: 'genLevelCtrl',
        type: ['commandMove', 'commandStop'],
        convert: (model, msg) => {
            const gesture = msg.type === 'commandMove' ? 'hold' : 'release';

            if (!threeButton) {
                return msg.endpoint.ID === 10 ? {action: gesture} : undefined;
            }

            return {action: `button_${msg.endpoint.ID - 9}_${gesture}`};
        },
    },
];

const createButtonExtend = (threeButton) => ({
    exposes: [
        e.action(threeButton
            ? [1, 2, 3].flatMap((button) =>
                gestures.map((gesture) => `button_${button}_${gesture}`))
            : gestures),
        e.numeric('voltage', exposes.access.STATE)
            .withUnit('mV')
            .withDescription('Voltage of the battery in millivolts')
            .withCategory('diagnostic'),
    ],

    fromZigbee: [
        ...gestureConverters(threeButton),
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
                // Unknown attributes must include their ZCL data type.
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
});

const createConverter = (modelID, threeButton) => ({
    fingerprint: [{
        modelID,
        manufacturerName: 'Broskie Applications',
    }],
    model: modelID,
    vendor: 'Broskie Applications',
    description: threeButton ? 'Zigbee three-button remote' : 'Zigbee single button',
    extend: [
        createButtonExtend(threeButton),
        m.battery(),
    ],
});

export default [
    createConverter('XIAO-Zigbee-Button', false),
    createConverter('XIAO-Zigbee-3Button', true),
];

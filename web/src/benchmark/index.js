const InfTestController = require('./controller/InfTestController');

const benchmarkApiConf = [
    //接口测试
    ['post', '/interface_test', InfTestController.interfaceDebug, {
        k8s: 'notEmpty'
    }],
    ['post', '/upload_tars_file', InfTestController.uploadTarsFile, {
        k8s: 'notEmpty'
    }],
    ['get', '/get_file_list', InfTestController.getFileList, {
        application: 'notEmpty',
        server_name: 'notEmpty',
        k8s: 'notEmpty'
    }],
    ['get', '/get_contexts', InfTestController.getContexts, {
        application: 'notEmpty',
        server_name: 'notEmpty',
        id: 'notEmpty',
        k8s: 'notEmpty'
    }],
    ['get', '/get_params', InfTestController.getParams, {
        application: 'notEmpty',
        server_name: 'notEmpty',
        id: 'notEmpty',
        module_name: 'notEmpty',
        interface_name: 'notEmpty',
        function_name: 'notEmpty',
        k8s: 'notEmpty'
    }],
    ['get', '/delete_tars_file', InfTestController.deleteTarsFile, {
        id: 'notEmpty'
    }],
    ['get', '/get_structs', InfTestController.getStructs, {
        id: 'notEmpty',
        module_name: 'notEmpty',
        k8s: 'notEmpty'
    }],

    //压力测试
    ['get', '/get_benchmark_des', InfTestController.getBenchmarkDes, {
        id: 'notEmpty',
        k8s: 'notEmpty'
    }],
    ['get', '/get_bm_case_list', InfTestController.getBmCaseList, {
        servant: 'notEmpty',
        fn: 'notEmpty',
        k8s: 'notEmpty'
    }],
    ['get', '/get_bm_result_by_id', InfTestController.getBmResultById, {
        id: 'notEmpty',
        k8s: 'notEmpty'
    }],
    ['post', '/upsert_bm_case', InfTestController.upsertBmCase, {
        servant: 'notEmpty',
        fn: 'notEmpty',
        k8s: 'notEmpty'
    }],
    ['post', '/delete_bm_case', InfTestController.deleteBmCase, {
        id: 'notEmpty',
        servant: 'notEmpty',
        k8s: 'notEmpty'
    }],

    ['post', '/start_benchmark', InfTestController.startBenchmark, {
        servant: 'notEmpty',
        fn: 'notEmpty',
        k8s: 'notEmpty'
    }],
    ['post', '/stop_benchmark', InfTestController.stopBenchmark, {
        servant: 'notEmpty',
        fn: 'notEmpty',
        k8s: 'notEmpty'

    }],
    ['post', '/test_benchmark', InfTestController.testBenchmark, {
        servant: 'notEmpty',
        fn: 'notEmpty',
        k8s: 'notEmpty'

    }],
    ['get', '/get_endpoints', InfTestController.getEndpoints, {
        servant: 'notEmpty',
        k8s: 'notEmpty'
    }],
    ['get', '/is_benchmark_installed', InfTestController.isBenchmarkInstalled, {
        k8s: 'notEmpty'
    }],
    // 测试用例
    ['post', '/interface_add_testcase', InfTestController.interfaceAddCase, {
        f_id: 'notEmpty',
        test_case_name: 'notEmpty',
        objName: 'notEmpty',
        file_name: 'notEmpty',
        module_name: 'notEmpty',
        interface_name: 'notEmpty',
        function_name: 'notEmpty',
        params: 'notEmpty',
        k8s: 'notEmpty'

    }],
    ['get', '/get_testcase_list', InfTestController.getTestCaseList, {
        f_id: 'notEmpty',
        k8s: 'notEmpty'

    }],
    ['get', '/delete_test_case', InfTestController.deleteTestCase, {
        case_id: 'notEmpty',
        k8s: 'notEmpty'

    }],
    ['post', '/modify_test_case', InfTestController.modifyTestCase, {
        case_id: 'notEmpty',
        test_case_name: 'notEmpty',
        params: 'notEmpty',
        k8s: 'notEmpty'
    }],
]

module.exports = {
    benchmarkApiConf
};
<template>
  <div class="page_server_debuger">
    <!-- tars文件列表 -->
    <wrapper
      ref="tarsFileListLoading"
      v-if="
        !showDebug &&
        !showBm &&
        !showCaseList &&
        !addTestCase &&
        !modifyTestCase
      "
    >
      <div class="table_head" style="height: 50px">
        <h4>
          {{ this.$t("inf.title.listTitle") }}
          <i
            class="el-icon-refresh-right"
            style="cursor: pointer"
            @click="getFileList()"
          ></i>
        </h4>

        <let-button
          size="small"
          theme="primary"
          class="add-btn"
          @click="openTarsUploadFileModal"
          >{{ $t("operate.add") }}</let-button
        >
      </div>

      <let-table :data="tarsFileList" :empty-msg="$t('common.nodata')">
        <let-table-column
          :title="$t('inf.benchmark.app')"
          prop="application"
        ></let-table-column>
        <let-table-column
          :title="$t('inf.benchmark.service')"
          prop="server_name"
        ></let-table-column>
        <let-table-column
          :title="$t('inf.benchmark.fileName')"
          prop="file_name"
        ></let-table-column>
        <let-table-column
          :title="$t('inf.benchmark.updateTime')"
          prop="posttime"
        ></let-table-column>
        <let-table-column :title="$t('operate.operates')" width="300px">
          <template slot-scope="scope">
            <let-table-operation @click="showDebuger(scope.row)">{{
              $t("inf.list.debug")
            }}</let-table-operation>
            <let-table-operation @click="showBenchmark(scope.row)">{{
              $t("inf.list.benchmark")
            }}</let-table-operation>
            <let-table-operation @click="getTestCaseList(scope.row)">{{
              $t("operate.testCaseList")
            }}</let-table-operation>
            <let-table-operation @click="deleteTarsFile(scope.row.f_id)">{{
              $t("operate.delete")
            }}</let-table-operation>
            <!-- <let-table-operation @click="gotoAddTestCase(scope.row)">{{$t('operate.addTestCase')}}</let-table-operation> -->
          </template>
        </let-table-column>
      </let-table>

      <el-divider></el-divider>
    </wrapper>

    <div v-if="showDebug">
      <let-form :inline="true">
        <let-form-item :label="$t('inf.dlg.selectLabel')">
          <let-cascader
            :data="contextData"
            required
            size="small"
            @change="getParams"
          ></let-cascader>
        </let-form-item>
        <let-form-item :label="$t('inf.dlg.objName')" v-if="objList.length">
          <let-select v-model="objName">
            <let-option
              v-for="item in objList"
              :value="item.servant"
              :key="item.servant"
            ></let-option>
          </let-select>
        </let-form-item>
        <let-form-item>
          <let-button theme="primary" @click="doDebug">{{
            $t("inf.list.debug")
          }}</let-button>
        </let-form-item>
      </let-form>

      <!-- 入参输入框 -->
      <let-row>
        <div class="params_container">
          <let-col :span="12" itemWidth="100%">
            <let-form itemWidth="100%">
              <let-input
                type="textarea"
                :rows="20"
                class="param_area div_line"
                v-model="inParam"
                :placeholder="$t('inf.dlg.inParam')"
              ></let-input>
            </let-form>
          </let-col>
          <let-col :span="12">
            <let-form itemWidth="100%">
              <let-input
                type="textarea"
                :rows="20"
                class="param_area"
                v-model="outParam"
                :placeholder="$t('inf.dlg.outParam')"
              ></let-input>
            </let-form>
          </let-col>
        </div>
      </let-row>

      <div class="mt10">
        <let-button theme="primary" size="small" @click="showDebug = false">{{
          $t("operate.goback")
        }}</let-button>
      </div>
    </div>
    <InterfaceBenchmark
      v-if="showBm"
      ref="bm"
      :servantList="objList"
    ></InterfaceBenchmark>
    <!-- 上传tars文件弹出框 -->
    <let-modal
      v-model="uploadModal.show"
      :title="$t('inf.title.dlgTitle')"
      width="880px"
      :footShow="false"
      @on-cancel="closeUploadModal"
    >
      <let-form
        v-if="uploadModal.model"
        ref="uploadForm"
        itemWidth="100%"
        @submit.native.prevent="uploadTarsFile"
      >
        <let-form-item itemWidth="400px">
          <let-uploader
            :placeholder="$t('inf.dlg.tarsFile')"
            accept=".tars"
            multiple
            @upload="uploadFile"
            >{{ $t("common.choose") }}</let-uploader
          >
          <li v-for="item in uploadModal.fileList2Show" :key="item.index">
            {{ item.name }}
          </li>
        </let-form-item>
        <let-button type="submit" theme="primary">{{
          $t("inf.dlg.upload")
        }}</let-button>
      </let-form>
    </let-modal>

    <div v-if="addTestCase && !showCaseList">
      <let-form :inline="true">
        <let-form-item :label="$t('inf.dlg.selectLabel')" required>
          <let-cascader
            :data="contextData"
            required
            size="small"
            @change="getParams"
          ></let-cascader>
        </let-form-item>
        <let-form-item
          :label="$t('inf.dlg.objName')"
          v-if="objList.length"
          required
        >
          <let-select v-model="objName">
            <let-option
              v-for="item in objList"
              :value="item.servant"
              :key="item.servant"
            ></let-option>
          </let-select>
        </let-form-item>
        <let-form-item :label="$t('inf.dlg.testCastName')" required>
          <let-input
            v-model="testCastName"
            :placeholder="$t('inf.dlg.testCastName')"
          ></let-input>
        </let-form-item>
      </let-form>

      <!-- 用例名字 -->
      <!-- <let-form class="left_align">
        <let-form-item :label="$t('inf.dlg.testCastName')">
          <let-input
            type="textarea"
            :rows="1"
            v-model="testCastName"
            :placeholder="$t('inf.dlg.testCastName')"
          ></let-input>
        </let-form-item>
      </let-form> -->

      <!-- 入参输入框 -->
      <let-row>
        <div class="params_container">
          <let-col itemWidth="100%">
            <let-form itemWidth="100%">
              <let-input
                type="textarea"
                :rows="20"
                class="param_area div_line"
                v-model="inParam"
                :placeholder="$t('inf.dlg.inParam')"
              ></let-input>
            </let-form>
          </let-col>
        </div>
      </let-row>

      <div class="mt10">
        <let-button theme="primary" @click="doAddTestCast">{{
          $t("operate.add")
        }}</let-button>
        &nbsp;&nbsp;
        <let-button
          theme="primary"
          @click="
            addTestCase = false;
            showCaseList = true;
          "
          >{{ $t("operate.goback") }}</let-button
        >
      </div>
    </div>

    <!-- 测试用例 -->
    <div v-if="showCaseList && !addTestCase">
      <let-form :inline="true">
        <let-form-item :label="$t('inf.dlg.selectLabel')">
          <let-cascader
            :data="contextData"
            required
            size="small"
            @change="getTestCaseByParams"
          ></let-cascader>
        </let-form-item>
        <let-form-item :label="$t('inf.dlg.objName')" v-if="objList.length">
          <let-select v-model="objName">
            <let-option
              v-for="item in objList"
              :value="item.servant"
              :key="item.servant"
            ></let-option>
          </let-select>
        </let-form-item>
      </let-form>

      <let-table
        :data="testCaseList"
        :title="$t('operate.testCaseList')"
        :empty-msg="$t('common.nodata')"
      >
        <let-table-column
          :title="$t('inf.dlg.testCastName')"
          prop="test_case_name"
          width="150px"
        ></let-table-column>
        <let-table-column
          :title="$t('inf.dlg.objName')"
          prop="object_name"
          width="200px"
        ></let-table-column>
        <let-table-column
          :title="$t('inf.dlg.fileName')"
          prop="file_name"
          width="150px"
        ></let-table-column>
        <let-table-column
          :title="$t('inf.dlg.module')"
          prop="module_name"
          width="100px"
        ></let-table-column>
        <let-table-column
          :title="$t('inf.dlg.method')"
          prop="function_name"
          width="100px"
        ></let-table-column>
        <let-table-column
          :title="$t('inf.dlg.user')"
          prop="modify_user"
          width="100px"
        ></let-table-column>
        <!-- <let-table-column
          :title="$t('board.alarm.table.content')"
          width="460px"
        >
          <template slot-scope="scope">
            <font color="red">{{ scope.row.context }}</font>
          </template>
        </let-table-column> -->
        <let-table-column :title="$t('operate.operates')" width="150px">
          <template slot-scope="scope">
            <let-table-operation @click="gotoModify(scope.row)">{{
              $t("operate.modify")
            }}</let-table-operation>
            <let-table-operation @click="deleteTestCase(scope.row.case_id)">{{
              $t("operate.delete")
            }}</let-table-operation>
          </template>
        </let-table-column>

        <let-table-column :title="$t('inf.dlg.node')">
          <template slot-scope="scope">
            <div v-for="(item, index) in totalServerList" :key="index">
              <let-table-operation @click="doNodedebug(item, scope.row)">{{
                item.node_name
              }}</let-table-operation>
              <br />
            </div>
          </template>
        </let-table-column>
      </let-table>
      <!-- <let-pagination
        :page="pageNum"
        @change="gotoPage"
        style="margin-bottom: 32px;"
        :total="total"
      ></let-pagination> -->
      <div class="mt10">
        <let-button theme="primary" size="small" @click="gotoAddTestCase()">{{
          $t("operate.addTestCase")
        }}</let-button>
        &nbsp;&nbsp;
        <let-button
          theme="primary"
          size="small"
          @click="showCaseList = false"
          >{{ $t("operate.goback") }}</let-button
        >
      </div>
    </div>

    <div v-if="modifyTestCase">
      <let-form class="left_align" itemWidth="530px">
        <let-form-item>
          <span class="text-blue"
            >{{ modifyCaseItem.object_name }}::{{
              modifyCaseItem.function_name
            }}</span
          >
        </let-form-item>
        <let-form-item :label="$t('inf.dlg.testCastName')">
          <let-input
            v-model="testCastName"
            :placeholder="$t('inf.dlg.testCastName')"
          ></let-input>
        </let-form-item>
      </let-form>

      <!-- 入参输入框 -->
      <let-row>
        <div class="params_container">
          <let-col itemWidth="100%">
            <let-form itemWidth="100%">
              <let-input
                type="textarea"
                :rows="20"
                class="param_area div_line"
                v-model="inParam"
                :placeholder="$t('inf.dlg.inParam')"
              ></let-input>
            </let-form>
          </let-col>
        </div>
      </let-row>

      <div class="mt10">
        <let-button theme="primary" @click="doModifyTestCase">{{
          $t("operate.modify")
        }}</let-button>
        &nbsp;&nbsp;
        <let-button
          theme="primary"
          @click="
            modifyTestCase = false;
            showCaseList = true;
          "
          >{{ $t("operate.goback") }}</let-button
        >
      </div>
    </div>

    <!--测试用例运行结果弹出框 -->
    <let-modal
      v-model="debugModal.show"
      :title="$t('inf.benchmark.debugResult')"
      width="600px"
      :footShow="false"
      @close="closeDebugModal"
    >
      <!-- 入参输入框 -->
      <let-row>
        <div class="params_container">
          <let-col itemWidth="100%">
            <let-form itemWidth="100%">
              <let-input
                type="textarea"
                :rows="40"
                class="param_area div_line"
                v-model="debugModal.resultParam"
                :placeholder="$t('inf.benchmark.debugResult')"
              ></let-input>
            </let-form>
          </let-col>
        </div>
      </let-row>
    </let-modal>
  </div>
</template>

<script>
import wrapper from "@/components/section-wrappper";
import InterfaceBenchmark from "./benchmark";
export default {
  name: "InterfaceDebuger",
  components: {
    wrapper,
    InterfaceBenchmark,
  },
  data() {
    return {
      k8s: false,
      // 当前页面信息
      serverData: {
        level: 5,
        application: "",
        server_name: "",
        set_name: "",
        set_area: "",
        set_group: "",
      },
      tarsFileList: [],
      totalServerList: [],

      uploadModal: {
        show: false,
        model: {},
        fileList2Show: [],
      },

      debugModal: {
        show: false,
        resultParam: "",
      },

      showDebug: false,
      showBm: false,
      // showInstallBm: false,
      // isBmInstalled: null,
      contextData: [],

      debuger_panel: false,
      inParam: "",
      outParam: "",
      selectedFileName: "",
      selectedMethods: [],
      objName: "",
      objList: [],
      selectedId: "",
      testCastName: "",
      testCaseList: [],
      showCaseList: false,
      // pageNum: 1,
      // pageSize: 20,
      // total: 0,
      addTestCase: false,
      modifyTestCase: false,
      modifyCaseItem: {},
      rowData: {},
    };
  },
  // props: ["treeid"],
  methods: {
    closeDebugModal() {
      this.debugModal.show = false;
    },
    getFileList() {
      const loading = this.$Loading.show();
      this.$ajax
        .getJSON("/api/get_file_list", {
          application: this.serverData.application,
          server_name: this.serverData.server_name,
          k8s: this.k8s,
        })
        .then((data) => {
          loading.hide();
          this.tarsFileList = data;
        })
        .catch((err) => {
          loading.hide();
          this.$tip.error(
            `${this.$t("common.error")}: ${err.message || err.err_msg}`
          );
        });
    },
    openTarsUploadFileModal() {
      this.uploadModal.show = true;
      this.uploadModal.model = {
        application: this.serverData.application,
        server_name: this.serverData.server_name,
        set_name: this.serverData.set_name,
        file: [],
      };
      this.uploadModal.fileList2Show = [];
    },
    uploadFile(fileList) {
      this.uploadModal.fileList2Show = [];
      if (fileList.length) {
        let len = 0;
        for (let file of fileList) {
          this.uploadModal.fileList2Show.push({
            name: file.name,
            index: Math.random() * 100,
          });
          const arr = file.name.split(".");
          const filetype = arr[arr.length - 1];
          // if (filetype === 'tars') {
          len += 1;
          // } else {
          //         break;
          //     }
        }
        if (len === fileList.length) {
          this.uploadModal.model.file = Array.from(fileList);
        }
      }
    },
    uploadTarsFile() {
      if (this.$refs.uploadForm.validate()) {
        const loading = this.$Loading.show();
        const formdata = new FormData();
        formdata.append("application", this.uploadModal.model.application);
        formdata.append("server_name", this.uploadModal.model.server_name);
        formdata.append("set_name", this.uploadModal.model.set_name);
        formdata.append("k8s", this.k8s);
        this.uploadModal.model.file.forEach((file) =>
          formdata.append("suse", file)
        );
        this.$ajax
          .postForm("/api/upload_tars_file", formdata)
          .then(() => {
            setTimeout(() => {
              loading.hide();
              this.getFileList();
              this.uploadModal.show = false;
              this.uploadModal.model = null;
            }, 1000);
          })
          .catch((err) => {
            loading.hide();
            this.$tip.error(
              `${this.$t("common.error")}: ${err.message || err.err_msg}`
            );
          });
      }
    },
    closeUploadModal() {
      this.uploadModal.show = false;
    },
    showDebuger(row) {
      this.showDebug = true;
      this.selectedFileName = row.file_name;
      this.inParam = null;
      this.outParam = null;
      this.selectedId = row.f_id;
      this.objName = null;
      this.getContextData(row.f_id);
      this.getObjList();
    },
    showBenchmark(row) {
      // if (this.isBmInstalled === null) return;
      // if (this.isBmInstalled) {
      this.showBm = true;
      this.showDebug = false;
      this.showCaseList = false;
      this.addTestCase = false;
      this.modifyTestCase = false;
      this.$nextTick(() => {
        this.$refs.bm.getBenchmarkDes(row.f_id);
      });
      // } else {
      //   this.showInstallBm = true;
      // }
    },
    deleteTarsFile(id) {
      this.$confirm(this.$t("inf.dlg.deleteMsg"), this.$t("common.alert"))
        .then(() => {
          const loading = this.$Loading.show();
          this.$ajax
            .getJSON("/api/delete_tars_file", {
              id: id,
              k8s: this.k8s,
            })
            .then((data) => {
              setTimeout(() => {
                loading.hide();
                this.getFileList();
              }, 1000);
            })
            .catch((err) => {
              loading.hide();
              this.$tip.error(
                `${this.$t("common.error")}: ${err.message || err.err_msg}`
              );
            });
        })
        .catch(() => {});
    },
    getObjList() {
      if (this.k8s) {
        this.$tars
          .getJSON("/k8s/api/all_adapter_conf_list", {
            ServerApp: this.serverData.application,
            ServerName: this.serverData.server_name,
          })
          .then((data) => {
            if (data.length) {
              // console.log(data);
              this.objList = data;
              this.objName = data[0].servant;
            }
          })
          .catch((err) => {
            this.$tip.error(
              `${this.$t("common.error")}: ${err.message || err.err_msg}`
            );
          });
      } else {
        this.$tars
          .getJSON("/server/api/all_adapter_conf_list", {
            application: this.serverData.application,
            server_name: this.serverData.server_name,
          })
          .then((data) => {
            if (data.length) {
              this.objList = data;
              this.objName = data[0].servant;
            }
          })
          .catch((err) => {
            this.$tip.error(
              `${this.$t("common.error")}: ${err.message || err.err_msg}`
            );
          });
      }
    },
    getContextData(id) {
      this.$ajax
        .getJSON("/api/get_contexts", {
          id: id,
          application: this.serverData.application,
          server_name: this.serverData.server_name,
          type: "all",
          k8s: this.k8s,
        })
        .then((data) => {
          this.contextData = data;
        })
        .catch((err) => {
          this.$tip.error(
            `${this.$t("common.error")}: ${err.message || err.err_msg}`
          );
        });
    },
    parseFields(fields) {
      let tmp = {};
      for (let item in fields) {
        let defaultVal = fields[item].defaultValue;
        if (!defaultVal) {
          if (fields[item].type === "string") {
            defaultVal = "";
          } else if (
            fields[item].type === "long" ||
            fields[item].type === "int"
          ) {
            defaultVal = 0;
          }
        }
        tmp[item] = defaultVal;
      }
      return tmp;
    },
    getParams(value) {
      this.selectedMethods = value;
      if (value.length == 3) {
        const loading = this.$Loading.show();
        this.$ajax
          .getJSON("/api/get_params", {
            application: this.serverData.application,
            server_name: this.serverData.server_name,
            id: this.selectedId,
            module_name: value[0],
            interface_name: value[1],
            function_name: value[2],
            k8s: this.k8s,
          })
          .then((data) => {
            loading.hide();
            let obj = {};
            this.$ajax
              .getJSON("/api/get_structs", {
                id: this.selectedId,
                module_name: value[0],
                k8s: this.k8s,
              })
              .then((contextData) => {
                data.forEach((item) => {
                  if (!item.out) {
                    if (item.type === "string") {
                      obj[item.name] = "";
                    } else if (item.type === "array") {
                      obj[item.name] = [];
                    } else if (typeof item.type === "object") {
                      if (item.type.vector) {
                        obj[item.name] = [];
                      } else if (item.type.isEnum) {
                        let tmp = {};
                        obj[item.name] = {};
                      } else if (item.type.isStruct) {
                        obj[item.name] = this.parseFields(
                          contextData.structs[item.type.name].fields
                        );
                      }
                    } else {
                      obj[item.name] = "";
                    }
                  }
                });
                this.inParam = JSON.stringify(obj, null, 4);
              });
          })
          .catch((err) => {
            loading.hide();
            this.$tip.error(
              `${this.$t("common.error")}: ${err.message || err.err_msg}`
            );
          });
      }
    },
    doDebug() {
      const loading = this.$Loading.show();
      this.$ajax
        .postJSON("/api/interface_test", {
          id: this.selectedId,
          k8s: this.k8s,
          application: this.serverData.application,
          server_name: this.serverData.server_name,
          file_name: this.selectedFileName,
          module_name: this.selectedMethods[0],
          interface_name: this.selectedMethods[1],
          function_name: this.selectedMethods[2],
          params: this.inParam,
          objName: this.objName,
        })
        .then((data) => {
          loading.hide();
          this.outParam = JSON.stringify(JSON.parse(data), null, 4);
        })
        .catch((err) => {
          loading.hide();
          this.$tip.error(
            `${this.$t("common.error")}: ${err.message || err.err_msg}`
          );
        });
    },
    // 删除测试用例
    deleteTestCase(case_id) {
      this.$confirm(this.$t("inf.dlg.deleteMsg"), this.$t("common.alert"))
        .then(() => {
          const loading = this.$Loading.show();
          this.$ajax
            .getJSON("/api/delete_test_case", {
              case_id: case_id,
              k8s: this.k8s,
            })
            .then((data) => {
              loading.hide();
              setTimeout(() => {
                this.getTestCaseListInner();
              }, 500);
            })
            .catch((err) => {
              loading.hide();
              this.$tip.error(
                `${this.$t("common.error")}: ${err.message || err.err_msg}`
              );
            });
        })
        .catch(() => {});
    },
    // 新增测试用例
    gotoAddTestCase() {
      this.addTestCase = true;
      this.showCaseList = false;
      this.selectedFileName = this.rowData.file_name;
      this.inParam = "";
      this.outParam = "";
      this.selectedId = this.rowData.f_id;
      this.objName = "";
      this.testCastName = "";
      this.selectedMethods = [];
      this.getContextData(this.rowData.f_id);
      this.getObjList();
    },
    // 获取文件对应测试用例列表
    getTestCaseListInner() {
      const loading = this.$Loading.show();
      this.showDebug = false;
      this.showCaseList = true;
      this.modifyTestCase = false;
      this.$ajax
        .getJSON("/api/get_testcase_list", {
          f_id: this.selectedId,
          application: this.serverData.application,
          server_name: this.serverData.server_name,
          // page_size: this.pageSize,
          // curr_page: curr_page,
          k8s: this.k8s,
        })
        .then((data) => {
          loading.hide();
          // this.pageNum = curr_page;
          // this.total = Math.ceil(data.count / this.pageSize);
          this.testCaseList = data.rows;
        })
        .catch((err) => {
          loading.hide();
          this.$tip.error(
            `${this.$t("get_testcase_list.failed")}: ${
              err.err_msg || err.message
            }`
          );
        });
    },
    // // 切换服务实时状态页码
    // gotoPage(num) {
    //   this.getTestCaseListInner(num);
    // },
    getTestCaseList(row) {
      this.rowData = row;
      this.showCaseList = true;
      this.testCaseList = [];
      this.selectedFileName = row.file_name;
      this.inParam = null;
      this.outParam = null;
      this.selectedId = row.f_id;
      this.objName = null;
      this.priorSet = null;
      this.selectedMethods = [];
      this.getContextData(row.f_id);
      this.getObjList();
      this.getTestCaseListInner(1);
    },
    getTestCaseByParams(value) {
      this.selectedMethods = value;
      if (value.length == 3) {
        this.getTestCaseListInner(1);
      }
    },
    doAddTestCast() {
      const loading = this.$Loading.show();
      this.$ajax
        .postJSON("/api/interface_add_testcase", {
          f_id: this.selectedId,
          application: this.serverData.application,
          server_name: this.serverData.server_name,
          file_name: this.selectedFileName,
          module_name: this.selectedMethods[0],
          interface_name: this.selectedMethods[1],
          function_name: this.selectedMethods[2],
          params: this.inParam,
          test_case_name: this.testCastName,
          objName: this.objName,
          k8s: this.k8s,
        })
        .then((data) => {
          loading.hide();
          this.addTestCase = false;
          setTimeout(() => {
            this.getTestCaseListInner();
          }, 500);
        })
        .catch((err) => {
          loading.hide();
          this.$tip.error(
            `${this.$t("common.error")}: ${err.message || err.err_msg}`
          );
        });
    },
    getServerList() {
      // 获取服务列表
      const loading = this.$Loading.show();
      if (this.k8s) {
        this.$tars
          .getJSON("/k8s/api/pod_list", {
            tree_node_id: this.treeid,
          })
          .then((data) => {
            loading.hide();
            const items = data.Data || [];
            items.forEach((item) => {
              item.isChecked = false;
              item.node_name = item.PodIp;
            });
            this.totalServerList = items;
            // console.log(this.totalServerList);
          })
          .catch((err) => {
            loading.hide();
            this.$confirm(
              err.err_msg || err.message || this.$t("serverList.table.msg.fail")
            ).then(() => {
              this.getServerList();
            });
          });
      } else {
        this.$tars
          .getJSON("/server/api/server_list", {
            tree_node_id: this.treeid,
          })
          .then((data) => {
            loading.hide();
            const items = data || [];
            items.forEach((item) => {
              item.isChecked = false;
            });
            this.totalServerList = items;
            // console.log(this.totalServerList);
          })
          .catch((err) => {
            loading.hide();
            this.$confirm(
              err.err_msg || err.message || this.$t("serverList.table.msg.fail")
            ).then(() => {
              this.getServerList();
            });
          });
      }
    },
    gotoModify(row) {
      this.modifyCaseItem = row;
      this.modifyTestCase = true;
      this.showCaseList = false;
      this.inParam = row.context;
      this.testCastName = row.test_case_name;
    },
    doModifyTestCase() {
      const loading = this.$Loading.show();
      this.$ajax
        .postJSON("/api/modify_test_case", {
          case_id: this.modifyCaseItem.case_id,
          params: this.inParam,
          test_case_name: this.testCastName,
          k8s: this.k8s,
        })
        .then((data) => {
          loading.hide();
          this.modifyTestCase = false;
          setTimeout(() => {
            this.getTestCaseListInner();
          }, 500);
        })
        .catch((err) => {
          loading.hide();
          this.$tip.error(
            `${this.$t("common.error")}: ${err.message || err.err_msg}`
          );
        });
    },
    doInterfaceTest(obj_endpoint, row) {
      //去到adapter信息  向特定节点请求服务
      this.$ajax
        .postJSON("/api/interface_test", {
          id: this.selectedId,
          application: this.serverData.application,
          server_name: this.serverData.server_name,
          file_name: this.selectedFileName,
          module_name: row.module_name,
          interface_name: row.interface_name,
          function_name: row.function_name,
          params: row.context,
          objName: row.object_name + "@" + obj_endpoint[row.object_name],
          k8s: this.k8s,
        })
        .then((data) => {
          // loading.hide();
          this.debugModal.show = true;
          this.debugModal.resultParam = JSON.stringify(
            JSON.parse(data),
            null,
            4
          );
        })
        .catch((err) => {
          // loading.hide();
          this.debugModal.show = true;
          this.debugModal.resultParam = err.message || err.err_msg;
        });
    },
    doNodedebug(server, row) {
      const loading = this.$Loading.show();

      if (this.k8s) {
        this.$tars
          .getJSON("/k8s/api/pod_list", {
            tree_node_id: this.treeid,
          })
          .then((adapterData) => {
            loading.hide();

            adapterData.Data.forEach((data) => {
              if (data.PodId != server.PodId) {
                return;
              }

              // node到endpoint的映射
              var obj_endpoint = {};
              for (let tmp_data in data.Servant) {
                console.log(tmp_data);
                obj_endpoint[tmp_data] = data.Servant[tmp_data];
              }

              console.log(obj_endpoint);

              this.doInterfaceTest(obj_endpoint, row);
              return;
            });
          })
          .catch((err) => {
            loading.hide();
            this.$tip.error(
              `${this.$t("serverList.restart.failed")}: ${
                err.err_msg || err.message
              }`
            );
          });
      } else {
        this.$tars
          .getJSON("/server/api/adapter_conf_list", {
            id: server.id,
          })
          .then((adapterData) => {
            loading.hide();
            // node到endpoint的映射
            var obj_endpoint = {};
            for (let tmp_data of adapterData) {
              obj_endpoint[tmp_data.servant] = tmp_data.endpoint;
            }

            this.doInterfaceTest(obj_endpoint, row);
          })
          .catch((err) => {
            loading.hide();
            this.$tip.error(
              `${this.$t("serverList.restart.failed")}: ${
                err.err_msg || err.message
              }`
            );
          });
      }
    },
    getParam(paramName) {
      var paramValue = null;
      var isFound = false;

      if (
        location.search.indexOf("?") == 0 &&
        location.search.indexOf("=") > 1
      ) {
        var arrSource = unescape(location.search)
          .substring(1, location.search.length)
          .split("&");
        var i = 0;
        while (i < arrSource.length && !isFound) {
          if (arrSource[i].indexOf("=") > 0) {
            if (
              arrSource[i].split("=")[0].toLowerCase() ==
              paramName.toLowerCase()
            ) {
              paramValue = arrSource[i].split("=")[1];
              isFound = true;
            }
          }
          i++;
        }
      }
      return paramValue;
    },
    getServerData() {
      const serverData = {
        level: 5,
        application: "",
        server_name: "",
        set_name: "",
        set_area: "",
        set_group: "",
      };
      if (!this.treeid) {
        return {};
      }

      const treeArr = this.treeid.split(".");

      treeArr.forEach((item) => {
        const level = +item.substr(0, 1);
        const name = item.substr(1);
        switch (level) {
          case 1:
            serverData.application = name;
            break;
          case 2:
            serverData.set_name = name;
            break;
          case 3:
            serverData.set_area = name;
            break;
          case 4:
            serverData.set_group = name;
            break;
          case 5:
            serverData.server_name = name;
            break;
          default:
            break;
        }
        serverData.level = level;
      });

      return serverData;
    },
  },
  created() {},
  mounted() {
    this.treeid = this.getParam("treeid");

    this.k8s = this.getParam("k8s") == "true";

    this.serverData = this.getServerData();

    this.getObjList();

    this.getFileList();
    this.getServerList();
  },
};
</script>

<style lang="postcss" scoped>
@import "../assets/css/variable.css";
.page_server_debuger {
  .add-btn {
    position: absolute;
    right: 0;
    top: 0;
    z-index: 2;
  }
  .mt10 {
    margin-top: 10px;
  }
  .param_area textarea {
    border: none !important;
  }
  .params_container {
    border: 1px solid #c0c4cc;
    overflow: hidden;
    border-radius: 5px;
  }
  .div_line {
    border-right: 1px solid #e4e9f2;
  }
  .left_align {
    margin-left: -15px;
  }
}
</style>

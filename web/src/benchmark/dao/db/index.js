/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

const Sequelize = require('sequelize');

const fs = require('fs-extra');

const _ = require('lodash');

const webConf = require('../../../config/webConf');

const logger = require('../../../logger');

let Db = {};

let databases = ['db_benchmark'];

databases.forEach((database) => {

	let {
		host,
		port,
		user,
		password,
		charset,
		pool,
	} = webConf.dbConf;

	const logging = process.env.NODE_ENV == "dev" ? (sqlText) => {
		// console.log(sqlText);

		// logger.sql(sqlText)
	} : true

	//初始化sequelize
	const sequelize = new Sequelize(database, user, password, {
		host,
		port,
		dialect: 'mysql',
		dialectOptions: {
			charset: charset
		},
		define: {
			charset: charset
		},
		logging,
		pool: {
			max: pool.max || 10,
			min: pool.min || 0,
			idle: pool.idle || 10000
		},
		timezone: (() => {
			let offset = 0 - new Date().getTimezoneOffset();
			return (offset >= 0 ? '+' : '-') + (Math.abs(parseInt(offset / 60)) + '').padStart(2, '0') + ':' + (offset % 60 + '').padStart(2, '0');
		})() //获取当前时区并做转换
	});

	let tableObj = {};
	let dbModelsPath = __dirname + '/' + database + '_models';
	let dbModels = fs.readdirSync(dbModelsPath);

	dbModels.forEach(async function (dbModel) {
		let tableName = dbModel.replace(/\.js$/g, '');
		tableObj[_.camelCase(tableName)] = sequelize.import(dbModelsPath + '/' + tableName);
	});

	Db[database] = tableObj;
	Db[database].sequelize = sequelize;

	// 测试是否连接成功
	(async function () {
		try {

			const s = new Sequelize("", user, password, {
				host,
				port,
				dialect: 'mysql',
				dialectOptions: {
					charset: charset
				},
				define: {
					charset: charset
				},
				logging,
				pool: {
					max: pool.max || 10,
					min: pool.min || 0,
					idle: pool.idle || 10000
				},
				timezone: (() => {
					let offset = 0 - new Date().getTimezoneOffset();
					return (offset >= 0 ? '+' : '-') + (Math.abs(parseInt(offset / 60)) + '').padStart(2, '0') + ':' + (offset % 60 + '').padStart(2, '0');
				})() //获取当前时区并做转换
			});

			await s.query(`CREATE DATABASE IF NOT EXISTS ${database};`);

			let connect = await sequelize.authenticate();
			console.log('Mysql connection has been established successfully.');

			if (webConf.webConf.alter) {
				try {

					for (let o in tableObj) {
						tableObj[o].sync({
							alter: true
						});
					}

				} catch (e) {
					console.log('database sync error:', e);
					process.exit(-1);
				}
			}

		} catch (err) {
			console.error('Mysql connection err', err)
		}
	})();
});

module.exports = Db;
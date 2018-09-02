#!groovy

env.'DependenciesBranch' = 'master'

def getBuildJobType() {
    return 'CI';
}

def buildVStudioSolutions(String msbuildPath, ArrayList solutions,
         String platform = 'Any CPU', String configuration = 'Release',
         Boolean restoreNuGet = false, Boolean updateNuGet = false,
         String mxCpuCount='/m', String verbosity = '/v:m', String target = '/t:Rebuild') {

    def errorCount = 0
    def buildExitCode = 0
    def index = 1
    def total = solutions.size()

    solutions.each { slnFile ->
         echo "******************** Building ($index of $total): $slnFile ********************"
         index = index + 1

        def buildParams = "/nologo ${target ?: ''} ${verbosity ?: ''} ${mxCpuCount ?: ''}"

        if (platform != null && platform != '') {
            buildParams = buildParams + " /p:Platform=\"${platform}\""
        }

        if (configuration != null && configuration != '') {
            buildParams = buildParams + " /p:Configuration=${configuration}"
        }

        //echo """Running => "${msbuildPath}" ${slnFile} ${buildParams}"""
        buildExitCode = bat returnStatus: true, script: "\"${msbuildPath}\" ${slnFile} ${buildParams}"

        echo  "----- MSBuild exit code = ${buildExitCode} -----"
        if (buildExitCode != 0) {
            errorCount = errorCount + 1
        }
    }

    def passed = total - errorCount
    echo "******************** Result => Passed=${passed}, Failed=${errorCount}, Total=${total} ********************"
    if (errorCount) {
        error 'Visual Studio solution(s) build failed'
        currentBuild.result = 'FAILED'
    }
}

def dep_repos = [
    'helloworld'
];

pipeline {
    agent {
        node {
            label 'test'
            customWorkspace "C:\\workspace\\vs_project_copy"
        }
    }
    options { timestamps(); timeout(time: 120, unit: 'MINUTES') }

    environment {
        msbuild = "${env.'ProgramFiles(x86)'}\\MSBuild\\14.0\\Bin\\MSBuild.exe"
        PATH = "${env.PATH};C:\\Program Files\\OpenCppCoverage"
    }

    stages {

        stage('Environment') {
            steps {

                bat "set"

            }
        }

        stage('Checkout Dependencies') {
            steps {
                script {
                    dep_repos.each {
                        // checkout it
                        echo 'checkout .. ${it} ..'
                    }
                }
            }
        }

        stage('Build') {
            steps {

                script {
                    if (getBuildJobType() == 'CI') {

                        buildVStudioSolutions("${msbuild}", ['src\\vs_project_copy.sln'], 'x86', 'Debug', false, false)

                    }
                    else if (getBuildJobType() ==~ /(IB|RC)/) {

                    }
                }

            }
        }

        stage('Test & Coverage') {
            when {
                expression { getBuildJobType() == 'CI' }
            }
            steps {

                echo 'ut/coverage test'

                // powershell "${env.WORKSPACE}/Develop/UnitTests/MakeCoverageReport.ps1 ${env.WORKSPACE} Debug Win32"

                // cobertura autoUpdateStability: true, coberturaReportFile: "CoverageReport.xml", conditionalCoverageTargets: '70, 0, 0', failUnhealthy: true, failUnstable: false, lineCoverageTargets: '80, 0, 0', maxNumberOfBuilds: 0, methodCoverageTargets: '80, 0, 0', onlyStable: false, sourceEncoding: 'ASCII', zoomCoverageChart: true

            }
        }

    }

    post {
        success {

            script {

                echo 'post/success/script ...'
            }

        }
        failure {
            script {

                echo 'post/failed/script ...'
            }
        }
    }
}


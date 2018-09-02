#!groovy

env.'DependenciesBranch' = 'master'

def getBuildJobType() {
    return 'CI';
}
def dep_repos = [
    'helloworld'
];

pipeline {
    agent {
        node {
            label 'any'
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
                dep_repos.each {
                    // checkout it
                    echo 'checkout .. ${it} ..'
                }
            }
        }

        stage('Build') {
            steps {

                script {
                    if (getBuildJobType() == 'CI') {

                        buildVStudioSolutions("${msbuild}", ['src\\vs_project_copy.sln'], 'Win32', 'Debug', false, false)

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
    }
}


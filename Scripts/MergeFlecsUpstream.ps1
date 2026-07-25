<#
.SYNOPSIS
Starts and validates an upstream Flecs merge without allowing generated headers
to be mistaken for the modular Flecs public header.

.DESCRIPTION
The default mode fetches the requested upstream branch, creates a backup branch,
and starts a no-commit merge with a 55 percent rename threshold.

After resolving conflicts, run the script again with -ValidateOnly. Validation
checks the modular flecs.h shape, forbidden upstream roots, upstream-to-plugin
path mappings, unresolved conflicts, whitespace errors, and newly added upstream
C++ tests that have not been converted to CQTest specs.

.EXAMPLE
.\Scripts\MergeFlecsUpstream.ps1

.EXAMPLE
.\Scripts\MergeFlecsUpstream.ps1 -ValidateOnly

.EXAMPLE
.\Scripts\MergeFlecsUpstream.ps1 -ValidateOnly -AllowMissingTestConversions
#>

[CmdletBinding()]
param(
	[string]$Remote = "upstream",
	[string]$Branch = "master",
	[ValidateRange(51, 100)]
	[int]$RenameThreshold = 55,
	[ValidateRange(1, 10485760)]
	[int]$MaxRootHeaderBytes = 307200,
	[switch]$ValidateOnly,
	[switch]$AllowMissingTestConversions
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Invoke-Git
{
	param(
		[Parameter(Mandatory)]
		[string[]]$Arguments,
		[switch]$AllowFailure
	)

	& git -c "safe.directory=$GitSafeRepositoryRoot" @Arguments
	$ExitCode = $LASTEXITCODE
	if ($ExitCode -ne 0 -and -not $AllowFailure)
	{
		throw "git $($Arguments -join ' ') failed with exit code $ExitCode."
	}

	return $ExitCode
}

function Get-GitOutput
{
	param(
		[Parameter(Mandatory)]
		[string[]]$Arguments,
		[switch]$AllowFailure
	)

	$Output = @(& git -c "safe.directory=$GitSafeRepositoryRoot" @Arguments)
	$ExitCode = $LASTEXITCODE
	if ($ExitCode -ne 0 -and -not $AllowFailure)
	{
		throw "git $($Arguments -join ' ') failed with exit code $ExitCode."
	}

	return $Output
}

function Test-GitRef
{
	param([Parameter(Mandatory)][string]$Ref)

	& git -c "safe.directory=$GitSafeRepositoryRoot" rev-parse --verify --quiet $Ref *> $null
	return $LASTEXITCODE -eq 0
}

function Get-ManifestCases
{
	param([Parameter(Mandatory)]$Manifest)

	$Cases = [System.Collections.Generic.HashSet[string]]::new(
		[System.StringComparer]::Ordinal
	)
	foreach ($Suite in $Manifest.test.testsuites)
	{
		foreach ($Case in $Suite.testcases)
		{
			[void]$Cases.Add("$($Suite.id)::$Case")
		}
	}

	return $Cases
}

function Get-GitJson
{
	param(
		[Parameter(Mandatory)][string]$Ref,
		[Parameter(Mandatory)][string]$Path
	)

	$Content = @(Get-GitOutput -Arguments @("show", "${Ref}:$Path") -AllowFailure)
	if ($LASTEXITCODE -ne 0 -or $Content.Count -eq 0)
	{
		return $null
	}

	return ($Content -join "`n") | ConvertFrom-Json
}

function Convert-UpstreamPath
{
	param([Parameter(Mandatory)][string]$UpstreamPath)

	$SpecialMappings = @{
		"src/addons/query_dsl/parser.c" = "Source/FlecsLibrary/Private/addons/query_dsl/query_dsl_parser.c"
		"src/addons/script/expr/ast.c" = "Source/FlecsLibrary/Private/addons/script/expr/ast_expr.c"
		"src/addons/script/expr/ast.h" = "Source/FlecsLibrary/Private/addons/script/expr/ast_expr.h"
		"src/addons/script/expr/expr.h" = "Source/FlecsLibrary/Private/addons/script/expr/expr_expr.h"
		"src/addons/script/expr/parser.c" = "Source/FlecsLibrary/Private/addons/script/expr/parser_expr.c"
		"src/addons/script/expr/stack.c" = "Source/FlecsLibrary/Private/addons/script/expr/stack_expr.c"
		"src/addons/script/expr/stack.h" = "Source/FlecsLibrary/Private/addons/script/expr/stack_expr.h"
		"src/addons/script/expr/util.c" = "Source/FlecsLibrary/Private/addons/script/expr/util_expr.c"
		"src/addons/script/expr/visit.h" = "Source/FlecsLibrary/Private/addons/script/expr/visit_expr.h"
		"src/addons/script/expr/visit_eval.c" = "Source/FlecsLibrary/Private/addons/script/expr/visit_eval_expr.c"
		"src/addons/script/expr/visit_fold.c" = "Source/FlecsLibrary/Private/addons/script/expr/visit_fold_expr.c"
		"src/addons/script/expr/visit_free.c" = "Source/FlecsLibrary/Private/addons/script/expr/visit_free_expr.c"
		"src/addons/script/expr/visit_to_str.c" = "Source/FlecsLibrary/Private/addons/script/expr/visit_to_str_expr.c"
		"src/addons/script/expr/visit_type.c" = "Source/FlecsLibrary/Private/addons/script/expr/visit_type_expr.c"
	}

	if ($SpecialMappings.ContainsKey($UpstreamPath))
	{
		return $SpecialMappings[$UpstreamPath]
	}
	if ($UpstreamPath.StartsWith("include/", [System.StringComparison]::Ordinal))
	{
		return "Source/FlecsLibrary/Public/" + $UpstreamPath.Substring(8)
	}
	if ($UpstreamPath.StartsWith("src/", [System.StringComparison]::Ordinal))
	{
		return "Source/FlecsLibrary/Private/" + $UpstreamPath.Substring(4)
	}

	return $null
}

function Test-MergeState
{
	param(
		[Parameter(Mandatory)][string]$TargetRef,
		[Parameter(Mandatory)][string]$PreviousUpstreamCommit
	)

	$Errors = [System.Collections.Generic.List[string]]::new()
	$Warnings = [System.Collections.Generic.List[string]]::new()

	$Unmerged = @(Get-GitOutput -Arguments @("diff", "--name-only", "--diff-filter=U"))
	if ($Unmerged.Count -gt 0)
	{
		$Errors.Add("Unresolved conflicts remain: $($Unmerged -join ', ')")
	}

	$ForbiddenPaths = @(Get-GitOutput -Arguments @("ls-files", "--", "include", "src", "distr", "test"))
	if ($ForbiddenPaths.Count -gt 0)
	{
		$Errors.Add("Upstream root paths are tracked in the plugin: $($ForbiddenPaths -join ', ')")
	}

	$HeaderPath = "Source/FlecsLibrary/Public/flecs.h"
	if (-not (Test-Path -LiteralPath $HeaderPath -PathType Leaf))
	{
		$Errors.Add("$HeaderPath is missing.")
	}
	else
	{
		$Header = Get-Content -LiteralPath $HeaderPath -Raw
		$PrivateIncludeCount = ([regex]::Matches(
			$Header,
			'(?m)^\s*#include\s+"flecs/private/api_defines\.h"\s*$'
		)).Count
		if ($PrivateIncludeCount -ne 1)
		{
			$Errors.Add("$HeaderPath must include flecs/private/api_defines.h exactly once; found $PrivateIncludeCount.")
		}
		if ($Header -match '(?m)^\s*#ifndef\s+FLECS_API_DEFINES_H\s*$')
		{
			$Errors.Add("$HeaderPath contains generated api_defines.h content.")
		}

		$HeaderBytes = (Get-Item -LiteralPath $HeaderPath).Length
		if ($HeaderBytes -gt $MaxRootHeaderBytes)
		{
			$Errors.Add("$HeaderPath is $HeaderBytes bytes; the modular-header limit is $MaxRootHeaderBytes.")
		}
	}

	$KnownUnmapped = [System.Collections.Generic.HashSet[string]]::new(
		[System.StringComparer]::Ordinal
	)
	[void]$KnownUnmapped.Add("include/flecs/addons/os_api_impl.h")
	$UpstreamFiles = @(Get-GitOutput -Arguments @(
		"ls-tree", "-r", "--name-only", $TargetRef, "--", "include", "src"
	))
	$MissingMappings = [System.Collections.Generic.List[string]]::new()
	foreach ($UpstreamPath in $UpstreamFiles)
	{
		if ($KnownUnmapped.Contains($UpstreamPath))
		{
			continue
		}

		$LocalPath = Convert-UpstreamPath -UpstreamPath $UpstreamPath
		if ($null -ne $LocalPath -and -not (Test-Path -LiteralPath $LocalPath -PathType Leaf))
		{
			$MissingMappings.Add("$UpstreamPath -> $LocalPath")
		}
	}
	if ($MissingMappings.Count -gt 0)
	{
		$Errors.Add("Missing mapped upstream files:`n  $($MissingMappings -join "`n  ")")
	}

	$CurrentManifestPath = "Source/FlecsLibrary/Tests/Specs/project.json"
	$TargetManifest = Get-GitJson -Ref $TargetRef -Path "test/cpp/project.json"
	$PreviousManifest = Get-GitJson -Ref $PreviousUpstreamCommit -Path "test/cpp/project.json"
	if ($null -eq $TargetManifest -or $null -eq $PreviousManifest)
	{
		$Warnings.Add("Could not compare upstream C++ test manifests.")
	}
	elseif (-not (Test-Path -LiteralPath $CurrentManifestPath -PathType Leaf))
	{
		$Errors.Add("$CurrentManifestPath is missing.")
	}
	else
	{
		$PreviousCases = Get-ManifestCases -Manifest $PreviousManifest
		$TargetCases = Get-ManifestCases -Manifest $TargetManifest
		$SpecSource = (Get-ChildItem "Source/FlecsLibrary/Tests/Specs" -Filter "*.spec.cpp" -File |
			Get-Content -Raw) -join "`n"
		$MissingNewCases = [System.Collections.Generic.List[string]]::new()
		foreach ($QualifiedCase in $TargetCases)
		{
			if ($PreviousCases.Contains($QualifiedCase))
			{
				continue
			}

			$Parts = $QualifiedCase.Split("::", 2)
			$FunctionName = "$($Parts[0])_$($Parts[1])"
			if ($SpecSource -notmatch "(?m)^\s*void\s+$([regex]::Escape($FunctionName))\s*\(")
			{
				$MissingNewCases.Add($QualifiedCase)
			}
		}

		if ($MissingNewCases.Count -gt 0)
		{
			$Message = "New upstream C++ tests lack CQTest conversions: $($MissingNewCases -join ', ')"
			if ($AllowMissingTestConversions)
			{
				$Warnings.Add($Message)
			}
			else
			{
				$Errors.Add($Message)
			}
		}
	}

	& git -c "safe.directory=$GitSafeRepositoryRoot" diff --cached --check
	if ($LASTEXITCODE -ne 0)
	{
		$Errors.Add("git diff --cached --check found whitespace errors.")
	}

	foreach ($Warning in $Warnings)
	{
		Write-Warning $Warning
	}
	foreach ($ValidationError in $Errors)
	{
		Write-Error -Message $ValidationError -ErrorAction Continue
	}

	if ($Errors.Count -gt 0)
	{
		throw "Merge validation failed with $($Errors.Count) error(s)."
	}

	Write-Host "Upstream merge validation passed." -ForegroundColor Green
}

$RepositoryRoot = Split-Path -Parent $PSScriptRoot
$GitSafeRepositoryRoot = $RepositoryRoot.Replace("\", "/")
Push-Location $RepositoryRoot
try
{
	$DetectedRoot = (@(Get-GitOutput -Arguments @("rev-parse", "--show-toplevel")))[0]
	if ([System.IO.Path]::GetFullPath($DetectedRoot) -ne [System.IO.Path]::GetFullPath($RepositoryRoot))
	{
		throw "Expected repository root '$RepositoryRoot', but Git reported '$DetectedRoot'."
	}

	$TargetRef = "$Remote/$Branch"
	if (-not $ValidateOnly)
	{
		if (@(Get-GitOutput -Arguments @("status", "--porcelain")).Count -gt 0)
		{
			throw "The worktree must be clean before starting an upstream merge."
		}
		if (Test-GitRef -Ref "MERGE_HEAD")
		{
			throw "A merge is already in progress."
		}
		if ((Get-GitOutput -Arguments @("remote")) -notcontains $Remote)
		{
			throw "Git remote '$Remote' does not exist."
		}

		Write-Host "Fetching $Remote $Branch..."
		Invoke-Git -Arguments @("fetch", $Remote, $Branch) | Out-Null
		if (-not (Test-GitRef -Ref $TargetRef))
		{
			throw "Fetched ref '$TargetRef' does not exist."
		}

		$PreviousUpstreamCommit = (@(Get-GitOutput -Arguments @("merge-base", "HEAD", $TargetRef)))[0]
		$BackupBranch = "backup/pre-flecs-merge-$(Get-Date -Format 'yyyyMMdd-HHmmss')"
		Invoke-Git -Arguments @("branch", $BackupBranch, "HEAD") | Out-Null
		Write-Host "Created backup branch $BackupBranch." -ForegroundColor Cyan

		Write-Host "Starting merge with rename threshold $RenameThreshold percent..."
		$MergeExitCode = Invoke-Git -Arguments @(
			"merge",
			"--no-commit",
			"--no-ff",
			"-Xfind-renames=$RenameThreshold%",
			$TargetRef
		) -AllowFailure
		if ($MergeExitCode -ne 0)
		{
			Write-Warning "Git stopped for conflict resolution. The backup is $BackupBranch."
			Write-Host "Resolve conflicts, then run .\Scripts\MergeFlecsUpstream.ps1 -ValidateOnly"
			Write-Host "To abandon the merge, run git merge --abort"
			exit $MergeExitCode
		}

		Write-Host "Merge applied without conflicts but has not been committed."
		Write-Host "Running validation..."
		Test-MergeState -TargetRef $TargetRef -PreviousUpstreamCommit $PreviousUpstreamCommit
		Write-Host "Review the staged merge and commit it when ready."
	}
	else
	{
		if (-not (Test-GitRef -Ref $TargetRef))
		{
			throw "Ref '$TargetRef' does not exist locally. Fetch it before validation."
		}

		$PreviousUpstreamCommit = (@(Get-GitOutput -Arguments @("merge-base", "HEAD", $TargetRef)))[0]
		Test-MergeState -TargetRef $TargetRef -PreviousUpstreamCommit $PreviousUpstreamCommit
	}
}
finally
{
	Pop-Location
}
